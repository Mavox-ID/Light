#define LT_INSTANCE_TYPE Instance
#include <light.h>
#include <shared/array.cpp>

// TODO Single instance.
// TODO Sorting lists.
// TODO Processes: handle/thread count; IO statistics; more memory information.

struct Instance : EsInstance {
	EsPanel *switcher;
	EsTextbox *textboxGeneralLog;
	EsListView *listViewProcesses;
	EsPanel *panelMemoryStatistics;
	int index;
	EsCommand commandTerminateProcess;
	Array<EsTextDisplay *> textDisplaysMemory;
};

#define REFRESH_INTERVAL (1000)

#define DISPLAY_PROCESSES (1)
#define DISPLAY_GENERAL_LOG (3)
#define DISPLAY_MEMORY (12)

#define PROCESSLT_COLUMN_NAME    (0)
#define PROCESSLT_COLUMN_PID     (1)
#define PROCESSLT_COLUMN_MEMORY  (2)
#define PROCESSLT_COLUMN_CPU     (3)
#define PROCESSLT_COLUMN_HANDLES (4)
#define PROCESSLT_COLUMN_THREADS (5)

const EsStyle styleMonospacedTextbox = {
	.inherit = LT_STYLE_TEXTBOX_NO_BORDER,

	.metrics = {
		.mask = LT_THEME_METRICS_FONT_FAMILY,
		.fontFamily = LT_FONT_MONOSPACED,
	},
};

const EsStyle stylePanelMemoryStatistics = {
	.inherit = LT_STYLE_PANEL_FILLED,

	.metrics = {
		.mask = LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_ALL,
		.insets = LT_RECT_1(8),
		.gapMajor = 5,
		.gapMinor = 5,
	},
};

const EsStyle stylePanelMemoryCommands = {
	.metrics = {
		.mask = LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_ALL,
		.insets = LT_RECT_4(0, 0, 0, 5),
		.gapMajor = 5,
		.gapMinor = 5,
	},
};

const char *pciClassCodeStrings[] = {
	"Unknown",
	"Mass storage controller",
	"Network controller",
	"Display controller",
	"Multimedia controller",
	"Memory controller",
	"Bridge controller",
	"Simple communication controller",
	"Base system peripheral",
	"Input device controller",
	"Docking station",
	"Processor",
	"Serial bus controller",
	"Wireless controller",
	"Intelligent controller",
	"Satellite communication controller",
	"Encryption controller",
	"Signal processing controller",
};

const char *pciSubclassCodeStrings1[] = {
	"SCSI bus controller",
	"IDE controller",
	"Floppy disk controller",
	"IPI bus controller",
	"RAID controller",
	"ATA controller",
	"Serial ATA",
	"Serial attached SCSI",
	"Non-volatile memory controller",
};

const char *pciSubclassCodeStrings12[] = {
	"FireWire (IEEE 1394) controller",
	"ACCESS bus",
	"SSA",
	"USB controller",
	"Fibre channel",
	"SMBus",
	"InfiniBand",
	"IPMI interface",
	"SERCOS interface (IEC 61491)",
	"CANbus",
};

const char *pciProgIFStrings12_3[] = {
	"UHCI",
	"OHCI",
	"EHCI",
	"XHCI",
};

struct ProcessItem {
	EsSnapshotProcessesItem data;
	uintptr_t cpuUsage;
};

char generalLogBuffer[256 * 1024];
Array<ProcessItem> processes;
int64_t selectedPID = -2;

ProcessItem *FindProcessByPID(Array<ProcessItem> snapshot, int64_t pid) {
	for (uintptr_t i = 0; i < snapshot.Length(); i++) {
		if (pid == snapshot[i].data.pid) {
			return &snapshot[i];
		}
	}

	return nullptr;
}

void UpdateProcesses(Instance *instance) {
	Array<ProcessItem> previous = processes;
	processes = {};

	size_t bufferSize;
	EsHandle handle = EsTakeSystemSnapshot(LT_SYSTEM_SNAPSHOT_PROCESSES, &bufferSize); 
	EsSnapshotProcesses *snapshot = (EsSnapshotProcesses *) EsHeapAllocate(bufferSize, false);
	EsConstantBufferRead(handle, snapshot);
	EsHandleClose(handle);

	for (uintptr_t i = 0; i < snapshot->count; i++) {
		ProcessItem item = {};
		item.data = snapshot->processes[i];
		processes.Add(item);

		if (snapshot->processes[i].isKernel) {
			ProcessItem item = {};
			item.data.cpuTimeSlices = snapshot->processes[i].idleTimeSlices;
			item.data.pid = -1;
			const char *idle = "CPU idle";
			item.data.nameBytes = EsCStringLength(idle);
			EsMemoryCopy(item.data.name, idle, item.data.nameBytes);
			processes.Add(item);
		}
	}

	EsHeapFree(snapshot);

	for (uintptr_t i = 0; i < previous.Length(); i++) {
		if (!FindProcessByPID(processes, previous[i].data.pid)) {
			bool found = EsListViewFixedItemRemove(instance->listViewProcesses, previous[i].data.pid);
			EsAssert(found);
			previous.Delete(i--);
		}
	}

	for (uintptr_t i = 0; i < processes.Length(); i++) {
		processes[i].cpuUsage = processes[i].data.cpuTimeSlices;
		ProcessItem *item = FindProcessByPID(previous, processes[i].data.pid);
		if (item) processes[i].cpuUsage -= item->data.cpuTimeSlices;
	}

	int64_t totalCPUTimeSlices = 0;

	for (uintptr_t i = 0; i < processes.Length(); i++) {
		totalCPUTimeSlices += processes[i].cpuUsage;
	}

	if (!totalCPUTimeSlices) {
		totalCPUTimeSlices = 1;
	}

	int64_t percentageSum = 0;

	for (uintptr_t i = 0; i < processes.Length(); i++) {
		processes[i].cpuUsage = processes[i].cpuUsage * 100 / totalCPUTimeSlices;
		percentageSum += processes[i].cpuUsage;
	}

	while (percentageSum < 100 && percentageSum) {
		for (uintptr_t i = 0; i < processes.Length(); i++) {
			if (processes[i].cpuUsage && percentageSum < 100) {
				processes[i].cpuUsage++, percentageSum++;
			}
		}
	}

	for (uintptr_t i = 0; i < processes.Length(); i++) {
		if (!FindProcessByPID(previous, processes[i].data.pid)) {
			EsListViewIndex index = EsListViewFixedItemInsert(instance->listViewProcesses, processes[i].data.pid);
			EsAssert(index == (EsListViewIndex) i);
		}
	}

	for (uintptr_t i = 0; i < processes.Length(); i++) {
		EsListViewFixedItemSetString (instance->listViewProcesses, i, PROCESSLT_COLUMN_NAME,    processes[i].data.name, processes[i].data.nameBytes);
		EsListViewFixedItemSetInteger(instance->listViewProcesses, i, PROCESSLT_COLUMN_PID,     processes[i].data.pid);
		EsListViewFixedItemSetInteger(instance->listViewProcesses, i, PROCESSLT_COLUMN_MEMORY,  processes[i].data.memoryUsage);
		EsListViewFixedItemSetInteger(instance->listViewProcesses, i, PROCESSLT_COLUMN_CPU,     processes[i].cpuUsage);
		EsListViewFixedItemSetInteger(instance->listViewProcesses, i, PROCESSLT_COLUMN_HANDLES, processes[i].data.handleCount);
		EsListViewFixedItemSetInteger(instance->listViewProcesses, i, PROCESSLT_COLUMN_THREADS, processes[i].data.threadCount);
	}

	EsListViewFixedItemSortAll(instance->listViewProcesses);
	EsCommandSetDisabled(&instance->commandTerminateProcess, selectedPID < 0 || !FindProcessByPID(processes, selectedPID));

	EsTimerSet(REFRESH_INTERVAL, [] (EsGeneric context) {
		Instance *instance = (Instance *) context.p;
		
		if (instance->index == DISPLAY_PROCESSES) {
			UpdateProcesses(instance);
		}
	}, instance); 

	previous.Free();
}

void UpdateDisplay(Instance *instance, int index) {
	instance->index = index;

	if (index != DISPLAY_PROCESSES) {
		EsCommandSetDisabled(&instance->commandTerminateProcess, true);
	}

	if (index == DISPLAY_PROCESSES) {
		UpdateProcesses(instance);
		EsPanelSwitchTo(instance->switcher, instance->listViewProcesses, LT_TRANSITION_NONE);
		EsElementFocus(instance->listViewProcesses);
	} else if (index == DISPLAY_GENERAL_LOG) {
		size_t bytes = _EsDebugCommand(index, (uintptr_t) generalLogBuffer, sizeof(generalLogBuffer), 0);
		EsTextboxSelectAll(instance->textboxGeneralLog);
		EsTextboxInsert(instance->textboxGeneralLog, generalLogBuffer, bytes);
		EsTextboxEnsureCaretVisible(instance->textboxGeneralLog, false);
		EsPanelSwitchTo(instance->switcher, instance->textboxGeneralLog, LT_TRANSITION_NONE);
	} else if (index == DISPLAY_MEMORY) {
		EsMemoryStatistics statistics = {};
		_EsDebugCommand(index, (uintptr_t) &statistics, 0, 0);

		EsPanelSwitchTo(instance->switcher, instance->panelMemoryStatistics, LT_TRANSITION_NONE);

		if (!instance->textDisplaysMemory.Length()) {
			EsPanel *panel = EsPanelCreate(instance->panelMemoryStatistics, LT_CELL_H_FILL | LT_PANEL_HORIZONTAL, EsStyleIntern(&stylePanelMemoryCommands));
			EsButton *button;

			button = EsButtonCreate(panel, LT_FLAGS_DEFAULT, 0, EsLiteral("Leak 1 MB"));
			EsButtonOnCommand(button, [] (Instance *, EsElement *, EsCommand *) { EsMemoryReserve(0x100000); });
			button = EsButtonCreate(panel, LT_FLAGS_DEFAULT, 0, EsLiteral("Leak 4 MB"));
			EsButtonOnCommand(button, [] (Instance *, EsElement *, EsCommand *) { EsMemoryReserve(0x400000); });
			button = EsButtonCreate(panel, LT_FLAGS_DEFAULT, 0, EsLiteral("Leak 16 MB"));
			EsButtonOnCommand(button, [] (Instance *, EsElement *, EsCommand *) { EsMemoryReserve(0x1000000); });
			button = EsButtonCreate(panel, LT_FLAGS_DEFAULT, 0, EsLiteral("Leak 64 MB"));
			EsButtonOnCommand(button, [] (Instance *, EsElement *, EsCommand *) { EsMemoryReserve(0x4000000); });
			button = EsButtonCreate(panel, LT_FLAGS_DEFAULT, 0, EsLiteral("Leak 256 MB"));
			EsButtonOnCommand(button, [] (Instance *, EsElement *, EsCommand *) { EsMemoryReserve(0x10000000); });

			EsSpacerCreate(instance->panelMemoryStatistics, LT_CELL_H_FILL);
		}

		char buffer[256];
		size_t bytes;
		uintptr_t index = 0;
#define ADD_MEMORY_STATISTIC_DISPLAY(label, ...) \
		bytes = EsStringFormat(buffer, sizeof(buffer), __VA_ARGS__); \
		if (instance->textDisplaysMemory.Length() == index) { \
			EsTextDisplayCreate(instance->panelMemoryStatistics, LT_CELL_H_PUSH | LT_CELL_H_RIGHT, 0, EsLiteral(label)); \
			instance->textDisplaysMemory.Add(EsTextDisplayCreate(instance->panelMemoryStatistics, LT_CELL_H_PUSH | LT_CELL_H_LEFT)); \
		} \
		EsTextDisplaySetContents(instance->textDisplaysMemory[index++], buffer, bytes)

		ADD_MEMORY_STATISTIC_DISPLAY("Fixed heap allocation count:", "%d", statistics.fixedHeapAllocationCount);
		ADD_MEMORY_STATISTIC_DISPLAY("Fixed heap graphics surfaces:", "%D (%d B)", 
				statistics.totalSurfaceBytes, statistics.totalSurfaceBytes);
		ADD_MEMORY_STATISTIC_DISPLAY("Fixed heap normal size:", "%D (%d B)", 
				statistics.fixedHeapTotalSize - statistics.totalSurfaceBytes, statistics.fixedHeapTotalSize - statistics.totalSurfaceBytes);
		ADD_MEMORY_STATISTIC_DISPLAY("Fixed heap total size:", "%D (%d B)", 
				statistics.fixedHeapTotalSize, statistics.fixedHeapTotalSize);
		ADD_MEMORY_STATISTIC_DISPLAY("Core heap allocation count:", "%d", statistics.coreHeapAllocationCount);
		ADD_MEMORY_STATISTIC_DISPLAY("Core heap total size:", "%D (%d B)", statistics.coreHeapTotalSize, statistics.coreHeapTotalSize);
		ADD_MEMORY_STATISTIC_DISPLAY("Cached boot FS nodes:", "%d", statistics.cachedNodes);
		ADD_MEMORY_STATISTIC_DISPLAY("Cached boot FS directory entries:", "%d", statistics.cachedDirectoryEntries);
		ADD_MEMORY_STATISTIC_DISPLAY("Maximum object cache size:", "%D (%d pages)", statistics.maximumObjectCachePages * LT_PAGE_SIZE, 
				statistics.maximumObjectCachePages);
		ADD_MEMORY_STATISTIC_DISPLAY("Approximate object cache size:", "%D (%d pages)", statistics.approximateObjectCacheSize, 
				statistics.approximateObjectCacheSize / LT_PAGE_SIZE);
		ADD_MEMORY_STATISTIC_DISPLAY("Commit (pageable):", "%D (%d pages)", statistics.commitPageable * LT_PAGE_SIZE, statistics.commitPageable);
		ADD_MEMORY_STATISTIC_DISPLAY("Commit (fixed):", "%D (%d pages)", statistics.commitFixed * LT_PAGE_SIZE, statistics.commitFixed);
		ADD_MEMORY_STATISTIC_DISPLAY("Commit (total):", "%D (%d pages)", (statistics.commitPageable + statistics.commitFixed) * LT_PAGE_SIZE, 
				statistics.commitPageable + statistics.commitFixed);
		ADD_MEMORY_STATISTIC_DISPLAY("Commit limit:", "%D (%d pages)", statistics.commitLimit * LT_PAGE_SIZE, statistics.commitLimit);
		ADD_MEMORY_STATISTIC_DISPLAY("Commit fixed limit:", "%D (%d pages)", statistics.commitFixedLimit * LT_PAGE_SIZE, statistics.commitFixedLimit);
		ADD_MEMORY_STATISTIC_DISPLAY("Commit remaining:", "%D (%d pages)", statistics.commitRemaining * LT_PAGE_SIZE, statistics.commitRemaining);

		ADD_MEMORY_STATISTIC_DISPLAY("Zeroed frames:", "%D (%d pages)", statistics.countZeroedPages * LT_PAGE_SIZE, statistics.countZeroedPages);
		ADD_MEMORY_STATISTIC_DISPLAY("Free frames:", "%D (%d pages)", statistics.countFreePages * LT_PAGE_SIZE, statistics.countFreePages);
		ADD_MEMORY_STATISTIC_DISPLAY("Standby frames:", "%D (%d pages)", statistics.countStandbyPages * LT_PAGE_SIZE, statistics.countStandbyPages);
		ADD_MEMORY_STATISTIC_DISPLAY("Active frames:", "%D (%d pages)", statistics.countActivePages * LT_PAGE_SIZE, statistics.countActivePages);

		EsTimerSet(REFRESH_INTERVAL, [] (EsGeneric context) {
			Instance *instance = (Instance *) context.p;

			if (instance->index == DISPLAY_MEMORY) {
				UpdateDisplay(instance, DISPLAY_MEMORY);
			}
		}, instance); 
	}
}

int ListViewProcessesCallback(EsElement *element, EsMessage *message) {
	if (message->type == LT_MSG_LIST_VIEW_SELECT && message->selectItem.isSelected) {
		selectedPID = processes[message->selectItem.index].data.pid;
		EsCommandSetDisabled(&element->instance->commandTerminateProcess, selectedPID < 0 || !FindProcessByPID(processes, selectedPID));
	} else {
		return 0;
	}

	return LT_HANDLED;
}

void AddTab(EsElement *toolbar, uintptr_t index, const char *label, bool asDefault = false) {
	EsButton *button = EsButtonCreate(toolbar, LT_BUTTON_RADIOBOX, 0, label);
	button->userData.u = index;

	EsButtonOnCommand(button, [] (Instance *instance, EsElement *element, EsCommand *) {
		if (EsButtonGetCheck((EsButton *) element) == LT_CHECK_CHECKED) {
			UpdateDisplay(instance, element->userData.u);
		}
	});

	if (asDefault) EsButtonSetCheck(button, LT_CHECK_CHECKED);
}

void TerminateProcess(Instance *instance, EsElement *, EsCommand *) {
	if (selectedPID == 0 /* Kernel */) {
		// Terminating the kernel process is a meaningless action; the closest equivalent is shutting down.
		EsSystemShowShutdownDialog();
		return;
	}

	// TODO What should happen if the user tries to terminate the desktop process?

	EsHandle handle = EsProcessOpen(selectedPID); 

	if (handle) {
		EsProcessTerminate(handle, 1); 
	} else {
		EsRectangle bounds = EsElementGetWindowBounds(instance->listViewProcesses);
		EsAnnouncementShow(instance->window, LT_FLAGS_DEFAULT, (bounds.l + bounds.r) / 2, (bounds.t + bounds.b) / 2, EsLiteral("Could not terminate process"));
	}
}

int InstanceCallback(Instance *, EsMessage *message) {
	if (message->type == LT_MSG_INSTANCE_DESTROY) {
		processes.Free();
	}

	return 0;
}

void ProcessApplicationMessage(EsMessage *message) {
	if (message->type == LT_MSG_INSTANCE_CREATE) {
		Instance *instance = EsInstanceCreate(message, "System Monitor");
		instance->callback = InstanceCallback;

		EsCommandRegister(&instance->commandTerminateProcess, instance, EsLiteral("Terminate process"), TerminateProcess, 1, "Del", false);

		EsWindow *window = instance->window;
		EsWindowSetIcon(window, LT_ICON_UTILITIES_SYSTEM_MONITOR);
		EsPanel *switcher = EsPanelCreate(window, LT_CELL_FILL | LT_PANEL_SWITCHER, LT_STYLE_PANEL_WINDOW_DIVIDER);
		instance->switcher = switcher;

		instance->textboxGeneralLog = EsTextboxCreate(switcher, LT_TEXTBOX_MULTILINE | LT_CELL_FILL | LT_ELEMENT_DISABLED, EsStyleIntern(&styleMonospacedTextbox));

		instance->listViewProcesses = EsListViewCreate(switcher, LT_CELL_FILL | LT_LIST_VIEW_COLUMNS | LT_LIST_VIEW_SINGLE_SELECT | LT_LIST_VIEW_FIXED_ITEMS);
		instance->listViewProcesses->messageUser = ListViewProcessesCallback;
		EsListViewRegisterColumn(instance->listViewProcesses, PROCESSLT_COLUMN_NAME, "Name", -1, LT_LIST_VIEW_COLUMN_HAS_MENU, 150);
		uint32_t numericColumnFlags = LT_LIST_VIEW_COLUMN_HAS_MENU | LT_TEXT_H_RIGHT | LT_DRAW_CONTENT_TABULAR 
			| LT_LIST_VIEW_COLUMN_DATA_INTEGERS | LT_LIST_VIEW_COLUMN_SORT_SIZE;
		EsListViewRegisterColumn(instance->listViewProcesses, PROCESSLT_COLUMN_PID, "PID", -1, numericColumnFlags, 120);
		EsListViewRegisterColumn(instance->listViewProcesses, PROCESSLT_COLUMN_MEMORY, "Memory", -1, numericColumnFlags | LT_LIST_VIEW_COLUMN_FORMAT_BYTES, 120);
		EsListViewRegisterColumn(instance->listViewProcesses, PROCESSLT_COLUMN_CPU, "CPU", -1, numericColumnFlags | LT_LIST_VIEW_COLUMN_FORMAT_PERCENTAGE, 120);
		EsListViewRegisterColumn(instance->listViewProcesses, PROCESSLT_COLUMN_HANDLES, "Handles", -1, numericColumnFlags, 120);
		EsListViewRegisterColumn(instance->listViewProcesses, PROCESSLT_COLUMN_THREADS, "Threads", -1, numericColumnFlags, 120);
		EsListViewAddAllColumns(instance->listViewProcesses);

		instance->panelMemoryStatistics = EsPanelCreate(switcher, 
				LT_CELL_FILL | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL | LT_PANEL_V_SCROLL_AUTO, EsStyleIntern(&stylePanelMemoryStatistics));
		EsPanelSetBands(instance->panelMemoryStatistics, 2 /* columns */);

		EsElement *toolbar = EsWindowGetToolbar(window);
		EsSpacerCreate(toolbar, LT_CELL_H_FILL);
		EsElement *buttonGroup = EsPanelCreate(toolbar, LT_PANEL_HORIZONTAL | LT_ELEMENT_AUTO_GROUP);
		AddTab(buttonGroup, DISPLAY_PROCESSES, "Processes", true);
		EsSpacerCreate(buttonGroup, LT_CELL_V_FILL, LT_STYLE_TOOLBAR_BUTTON_GROUP_SEPARATOR);
		AddTab(buttonGroup, DISPLAY_GENERAL_LOG, "System log");
		EsSpacerCreate(buttonGroup, LT_CELL_V_FILL, LT_STYLE_TOOLBAR_BUTTON_GROUP_SEPARATOR);
		AddTab(buttonGroup, DISPLAY_MEMORY, "Memory");
		EsSpacerCreate(toolbar, LT_CELL_H_FILL);
	}
}

void _start() {
	_init();

	while (true) {
		ProcessApplicationMessage(EsMessageReceive());
	}
}
