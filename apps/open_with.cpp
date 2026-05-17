#define LT_INSTANCE_TYPE Instance
#include <light.h>
#include <shared/strings.cpp>

struct Instance : EsInstance {
	char *pendingFilePath;
	size_t pendingFilePathBytes;
};

void ProcessApplicationMessage(EsMessage *message) {
	if (message->type != LT_MSG_INSTANCE_CREATE) return;

	Instance *instance = EsInstanceCreate(message, EsLiteral("Open With"));
	EsWindowSetIcon(instance->window, LT_ICON_APPLICATION_DEFAULT_ICON);
	EsWindowSetTitle(instance->window, EsLiteral("Open file as..."));

	EsApplicationStartupRequest startup = EsInstanceGetStartupRequest(instance);

	if (!startup.filePathBytes) {
		EsInstanceClose(instance);
		return;
	}

	instance->pendingFilePath = (char *) EsHeapAllocate(startup.filePathBytes, false);
	EsMemoryCopy(instance->pendingFilePath, startup.filePath, startup.filePathBytes);
	instance->pendingFilePathBytes = startup.filePathBytes;

	EsPanel *root = EsPanelCreate(instance->window, LT_CELL_FILL | LT_PANEL_VERTICAL, LT_STYLE_PANEL_WINDOW_BACKGROUND);
	EsTextDisplayCreate(root, LT_CELL_H_FILL, LT_STYLE_TEXT_LABEL, EsLiteral("Select an application:"));
	EsSpacerCreate(root, LT_CELL_H_FILL, 0, 0, 8);

	EsPanel *appList = EsPanelCreate(root, LT_CELL_FILL | LT_PANEL_VERTICAL | LT_PANEL_SCROLL_Y, LT_STYLE_PANEL_INSET);
	appList->separatorStylePart = LT_STYLE_BUTTON_GROUP_SEPARATOR;
	appList->separatorFlags = LT_CELL_H_FILL;

	size_t configBytes;
	char *configData = (char *) EsFileReadAll(EsLiteral("0:/Light/Default.ini"), &configBytes);

	if (configData) {
		EsINIState s = { .buffer = configData, .bytes = configBytes };

		char *appName = nullptr; size_t appNameBytes = 0;
		char *appExec = nullptr; size_t appExecBytes = 0;
		int64_t appId = 0;
		bool appHidden = false;
		bool inAppSection = false;

		auto CreateButton = [&]() {
			if (!inAppSection || !appName || !appExec || appHidden || appId <= 0) return;

			EsButton *btn = EsButtonCreate(appList,
				LT_CELL_H_FILL | LT_ELEMENT_NO_FOCUS_ON_CLICK,
				LT_STYLE_BUTTON_GROUP_ITEM,
				appName, (ptrdiff_t) appNameBytes);
			btn->userData.i = (intptr_t) appId;

			size_t lppSize;
			void *lppData = EsFileMap(appExec, appExecBytes, &lppSize, LT_MEMORY_MAP_OBJECT_READ_ONLY);
			if (lppData) {
				EsBundle bundle = { .base = (const BundleHeader *) lppData, .bytes = (ptrdiff_t) lppSize };
				size_t iconBytes;
				const void *icon = EsBundleFind(&bundle, EsLiteral("$Icons/32"), &iconBytes);
				if (icon) {
					uint32_t w, h;
					uint32_t *bits = (uint32_t *) EsImageLoad(icon, iconBytes, &w, &h, 4);
					if (bits) {
						EsButtonSetIconFromBits(btn, bits, w, h, w * 4);
						EsHeapFree(bits);
					} else {
						EsButtonSetIcon(btn, LT_ICON_APPLICATION_DEFAULT_ICON);
					}
				} else {
					EsButtonSetIcon(btn, LT_ICON_APPLICATION_DEFAULT_ICON);
				}
				EsMemoryUnreserve(lppData);
			} else {
				EsButtonSetIcon(btn, LT_ICON_APPLICATION_DEFAULT_ICON);
			}

			EsButtonOnCommand(btn, [] (Instance *instance, EsElement *element, EsCommand *) {
				EsApplicationStartupRequest req = {};
				req.id = (int64_t) element->userData.i;
				req.filePath = instance->pendingFilePath;
				req.filePathBytes = instance->pendingFilePathBytes;
				EsApplicationStart(instance, &req);
				EsInstanceClose(instance);
			});
		};

		while (EsINIParse(&s)) {
			if (!s.keyBytes) {
				CreateButton();
				inAppSection = (0 == EsStringCompareRaw(s.section, s.sectionBytes, EsLiteral("application")));
				appName = nullptr; appNameBytes = 0;
				appExec = nullptr; appExecBytes = 0;
				appId = 0; appHidden = false;
			} else if (inAppSection) {
				if      (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("name")))       { appName   = s.value; appNameBytes = s.valueBytes; }
				else if (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("executable"))) { appExec   = s.value; appExecBytes = s.valueBytes; }
				else if (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("id")))         { appId     = EsIntegerParse(s.value, s.valueBytes); }
				else if (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("hidden")))     { appHidden = EsIntegerParse(s.value, s.valueBytes) != 0; }
			}
		}
		CreateButton();
		EsHeapFree(configData);
	}

	EsButton *runBtn = EsButtonCreate(appList,
		LT_CELL_H_FILL | LT_ELEMENT_NO_FOCUS_ON_CLICK,
		LT_STYLE_BUTTON_GROUP_ITEM,
		EsLiteral("Run as executable"));
	EsButtonSetIcon(runBtn, LT_ICON_APPLICATION_DEFAULT_ICON);
	EsButtonOnCommand(runBtn, [] (Instance *instance, EsElement *, EsCommand *) {
		EsApplicationRunTemporary(instance->pendingFilePath, instance->pendingFilePathBytes);
		EsInstanceClose(instance);
	});

	EsSpacerCreate(root, LT_CELL_H_FILL, 0, 0, 8);
	EsPanel *footer = EsPanelCreate(root, LT_CELL_H_FILL | LT_PANEL_HORIZONTAL, 0);
	EsSpacerCreate(footer, LT_CELL_H_FILL);
	EsButton *cancelBtn = EsButtonCreate(footer, LT_FLAGS_DEFAULT, 0, EsLiteral("Cancel"));
	EsButtonOnCommand(cancelBtn, [] (Instance *instance, EsElement *, EsCommand *) {
		EsInstanceClose(instance);
	});
}

void _start() {
	_init();
	while (true) {
		ProcessApplicationMessage(EsMessageReceive());
	}
}
