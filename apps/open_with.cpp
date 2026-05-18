#define LT_INSTANCE_TYPE Instance
#include <light.h>
#include <shared/strings.cpp>
#include <shared/crc.h>

struct LppBundleHeader {
	uint32_t signature; // 0x63BDAF45
	uint32_t version;   // 1
	uint32_t fileCount;
	uint32_t _unused;
	uint64_t mapAddress;
};

struct LppBundleFile {
	uint64_t nameCRC64;
	uint64_t bytes;
	uint64_t offset;
};

static const void *LppFindIcon32(const void *data, size_t dataSize, size_t *outBytes) {
	if (!data || dataSize < sizeof(LppBundleHeader)) return nullptr;
	const LppBundleHeader *header = (const LppBundleHeader *) data;
	if (header->signature != 0x63BDAF45 || header->version != 1) return nullptr;
	size_t filesSize = (size_t) header->fileCount * sizeof(LppBundleFile);
	if (dataSize < sizeof(LppBundleHeader) + filesSize) return nullptr;
	const LppBundleFile *files = (const LppBundleFile *)(header + 1);
	uint64_t targetCRC = CalculateCRC64("$Icons/32", 9, 0);
	for (uint32_t i = 0; i < header->fileCount; i++) {
		if (files[i].nameCRC64 == targetCRC) {
			if (files[i].offset + files[i].bytes > dataSize) return nullptr;
			if (outBytes) *outBytes = (size_t) files[i].bytes;
			return (const uint8_t *) data + files[i].offset;
		}
	}
	return nullptr;
}

struct Instance : EsInstance {
	char *pendingFilePath;
	size_t pendingFilePathBytes;
};

void ProcessApplicationMessage(EsMessage *message) {
	if (message->type != LT_MSG_INSTANCE_CREATE) return;

	Instance *instance = EsInstanceCreate(message, EsLiteral("Open With"));
	EsWindowSetTitle(instance->window, EsLiteral("Open file as..."));
	EsWindowSetIcon(instance->window, LT_ICON_APPLICATION_DEFAULT_ICON);

	EsApplicationStartupRequest startup = EsInstanceGetStartupRequest(instance);

	if (!startup.filePathBytes) {
		EsInstanceClose(instance);
		return;
	}

	instance->pendingFilePath = (char *) EsHeapAllocate(startup.filePathBytes, false);
	EsMemoryCopy(instance->pendingFilePath, startup.filePath, startup.filePathBytes);
	instance->pendingFilePathBytes = startup.filePathBytes;

	EsPanel *root = EsPanelCreate(instance->window, LT_CELL_FILL | LT_PANEL_VERTICAL,
	                               LT_STYLE_PANEL_WINDOW_BACKGROUND);
	EsTextDisplayCreate(root, LT_CELL_H_FILL, LT_STYLE_TEXT_LABEL, EsLiteral("Choose an application:"));
	EsSpacerCreate(root, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);

	EsPanel *appList = EsPanelCreate(root, LT_CELL_FILL | LT_PANEL_VERTICAL | LT_PANEL_V_SCROLL_AUTO,
	                                  LT_STYLE_PANEL_INSET);

	size_t configBytes;
	char *configData = (char *) EsFileReadAll(EsLiteral("0:/Light/Default.ini"), &configBytes);

	if (configData) {
		EsINIState s = { .buffer = configData, .bytes = configBytes };

		char *appName  = nullptr; size_t appNameBytes  = 0;
		char *appExec  = nullptr; size_t appExecBytes  = 0;
		int64_t appId  = 0;
		bool appHidden = false;
		bool inApp     = false;

		auto FlushApp = [&]() {
			if (!inApp || !appName || !appExec || appHidden || appId <= 0) return;

			EsButton *btn = EsButtonCreate(appList,
				LT_CELL_H_FILL | LT_ELEMENT_NO_FOCUS_ON_CLICK,
				LT_STYLE_BUTTON_GROUP_ITEM,
				appName, (ptrdiff_t) appNameBytes);
			btn->userData.i = (intptr_t) appId;

			size_t lppSize;
			void *lppData = EsFileMap(appExec, (ptrdiff_t) appExecBytes, &lppSize,
			                          LT_MEMORY_MAP_OBJECT_READ_ONLY);
			if (lppData) {
				size_t icon32Bytes;
				const void *icon32 = LppFindIcon32(lppData, lppSize, &icon32Bytes);
				if (icon32) {
					uint32_t w, h;
					uint32_t *bits = (uint32_t *) EsImageLoad(icon32, icon32Bytes, &w, &h, 4);
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
				req.id          = (int64_t) element->userData.i;
				req.filePath      = instance->pendingFilePath;
				req.filePathBytes = (ptrdiff_t) instance->pendingFilePathBytes;
				EsApplicationStart(instance, &req);
				EsInstanceClose(instance);
			});
		};

		while (EsINIParse(&s)) {
			if (!s.keyBytes) {
				FlushApp();
				inApp     = (0 == EsStringCompareRaw(s.section, s.sectionBytes, EsLiteral("application")));
				appName   = nullptr; appNameBytes  = 0;
				appExec   = nullptr; appExecBytes  = 0;
				appId     = 0;       appHidden     = false;
			} else if (inApp) {
				if      (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("name")))
					{ appName   = s.value; appNameBytes  = s.valueBytes; }
				else if (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("executable")))
					{ appExec   = s.value; appExecBytes  = s.valueBytes; }
				else if (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("id")))
					{ appId     = EsIntegerParse(s.value, s.valueBytes); }
				else if (0 == EsStringCompareRaw(s.key, s.keyBytes, EsLiteral("hidden")))
					{ appHidden = EsIntegerParse(s.value, s.valueBytes) != 0; }
			}
		}
		FlushApp();
		EsHeapFree(configData);
	}

	EsSpacerCreate(appList, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);
	EsButton *runBtn = EsButtonCreate(appList,
		LT_CELL_H_FILL | LT_ELEMENT_NO_FOCUS_ON_CLICK,
		LT_STYLE_BUTTON_GROUP_ITEM,
		EsLiteral("Run as executable"));
	EsButtonSetIcon(runBtn, LT_ICON_APPLICATION_DEFAULT_ICON);
	EsButtonOnCommand(runBtn, [] (Instance *instance, EsElement *, EsCommand *) {
		EsApplicationRunTemporary(instance->pendingFilePath,
		                          (ptrdiff_t) instance->pendingFilePathBytes);
		EsInstanceClose(instance);
	});

	EsSpacerCreate(root, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);
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
