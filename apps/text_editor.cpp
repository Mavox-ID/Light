#define LT_INSTANCE_TYPE Instance
#include <light.h>

#include <shared/strings.cpp>

// TODO Document save/load model, then merge into API.
// TODO Replace toolbar, then merge into API.
// TODO Merge Format menu into API.
// TODO Word wrap (textbox feature).

// TODO Possible extension features:
// - Block selection
// - Folding
// - Tab settings and auto-indent
// - Macros
// - Status bar
// - Goto line
// - Find in files
// - Convert case
// - Sort lines
// - Trim trailing space
// - Indent/comment/join/split shortcuts

#define SETTINGS_FILE "|Settings:/Default.ini"

const EsInstanceClassEditorSettings editorSettings = {
	INTERFACE_STRING(TextEditorNewFileName),
	INTERFACE_STRING(TextEditorNewDocument),
	LT_ICON_TEXT,
};

const EsStyle styleFormatPopupColumn = {
	.metrics = {
		.mask = LT_THEME_METRICS_GAP_MAJOR,
		.gapMajor = 5,
	},
};

struct Instance : EsInstance {
	EsTextbox *textboxDocument,
		  *textboxSearch;

	EsElement *toolbarMain, *toolbarSearch;

	EsTextDisplay *displaySearch;

	EsButton *buttonFormat;

	EsCommand commandFindNext,
		  commandFindPrevious,
		  commandFind,
		  commandFormat,
		  commandZoomIn,
		  commandZoomOut;

	uint32_t syntaxHighlightingLanguage;
	int32_t textSize;
	int32_t scrollCumulative;
};

const int presetTextSizes[] = {
	8, 9, 10, 11, 12, 13,
	14, 16,
	18, 24, 30,
	36, 48, 60,
	72, 96, 120, 144,
};

int32_t globalTextSize = 10;

void Find(Instance *instance, bool backwards) {
	EsWindowSwitchToolbar(instance->window, instance->toolbarSearch, LT_TRANSITION_SLIDE_UP);

	size_t needleBytes;
	char *needle = EsTextboxGetContents(instance->textboxSearch, &needleBytes);

	int32_t line0, byte0, line1, byte1;
	EsTextboxGetSelection(instance->textboxDocument, &line0, &byte0, &line1, &byte1);

	if (backwards) {
		if (line1 < line0) {
			line0 = line1;
			byte0 = byte1;
		} else if (line1 == line0 && byte1 < byte0) {
			byte0 = byte1;
		}
	} else {
		if (line1 > line0) {
			line0 = line1;
			byte0 = byte1;
		} else if (line1 == line0 && byte1 > byte0) {
			byte0 = byte1;
		}
	}

	bool found = EsTextboxFind(instance->textboxDocument, needle, needleBytes, &line0, &byte0, backwards ? LT_TEXTBOX_FIND_BACKWARDS : LT_FLAGS_DEFAULT);

	if (found) {
		EsTextDisplaySetContents(instance->displaySearch, "");
		EsTextboxSetSelection(instance->textboxDocument, line0, byte0, line0, byte0 + needleBytes);
		EsTextboxEnsureCaretVisible(instance->textboxDocument, true);
		EsElementFocus(instance->textboxDocument);
	} else if (!needleBytes) {
		EsTextDisplaySetContents(instance->displaySearch, INTERFACE_STRING(CommonSearchPrompt2));
		EsElementFocus(instance->textboxSearch);
	} else {
		EsTextDisplaySetContents(instance->displaySearch, INTERFACE_STRING(CommonSearchNoMatches));
		EsElementFocus(instance->textboxSearch);
	}

	EsHeapFree(needle);
}

void SetLanguage(Instance *instance, uint32_t newLanguage) {
	EsFont font = {};
	font.family = newLanguage ? LT_FONT_MONOSPACED : LT_FONT_SANS;
	font.weight = LT_FONT_REGULAR;
	EsTextboxSetFont(instance->textboxDocument, font);

	instance->syntaxHighlightingLanguage = newLanguage;
	EsTextboxSetupSyntaxHighlighting(instance->textboxDocument, newLanguage);
	EsTextboxEnableSmartReplacement(instance->textboxDocument, !newLanguage);
}

void FormatPopupCreate(Instance *instance) {
	EsMenu *menu = EsMenuCreate(instance->buttonFormat, LT_FLAGS_DEFAULT);
	EsPanel *panel = EsPanelCreate(menu, LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_POPUP);
	
	{
		EsPanel *column = EsPanelCreate(panel, LT_FLAGS_DEFAULT, EsStyleIntern(&styleFormatPopupColumn));
		EsTextDisplayCreate(column, LT_CELL_H_EXPAND, LT_STYLE_TEXT_LABEL, INTERFACE_STRING(CommonFormatSize));
		EsListView *list = EsListViewCreate(column, LT_LIST_VIEW_CHOICE_SELECT | LT_LIST_VIEW_FIXED_ITEMS, LT_STYLE_LIST_CHOICE_BORDERED);

		size_t presetSizeCount = sizeof(presetTextSizes) / sizeof(presetTextSizes[0]);
		int currentSize = instance->textSize;
		char buffer[64];

		if (currentSize < presetTextSizes[0]) {
			// The current size is not in the list; add it.
			EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, currentSize), 0, buffer, 
					EsStringFormat(buffer, sizeof(buffer), "%d pt", currentSize));
		}

		for (uintptr_t i = 0; i < presetSizeCount; i++) {
			EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, presetTextSizes[i]), 0, buffer, 
					EsStringFormat(buffer, sizeof(buffer), "%d pt", presetTextSizes[i]));

			if (currentSize > presetTextSizes[i] && (i == presetSizeCount - 1 || (i != presetSizeCount - 1 && currentSize < presetTextSizes[i + 1]))) {
				// The current size is not in the list; add it.
				EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, currentSize), 0, buffer, 
						EsStringFormat(buffer, sizeof(buffer), "%d pt", currentSize));
			}
		}

		EsListViewFixedItemSelect(list, currentSize);

		list->messageUser = [] (EsElement *element, EsMessage *message) {
			if (message->type == LT_MSG_LIST_VIEW_SELECT) {
				Instance *instance = element->instance;
				EsGeneric newSize;

				if (EsListViewFixedItemGetSelected(((EsListView *) element), &newSize)) {
					globalTextSize = instance->textSize = newSize.u;
					EsTextboxSetTextSize(instance->textboxDocument, instance->textSize);
				}
			}

			return 0;
		};
	}

	{
		EsPanel *column = EsPanelCreate(panel, LT_FLAGS_DEFAULT, EsStyleIntern(&styleFormatPopupColumn));
		EsTextDisplayCreate(column, LT_CELL_H_EXPAND, LT_STYLE_TEXT_LABEL, INTERFACE_STRING(CommonFormatLanguage));
		EsListView *list = EsListViewCreate(column, LT_LIST_VIEW_CHOICE_SELECT | LT_LIST_VIEW_FIXED_ITEMS, LT_STYLE_LIST_CHOICE_BORDERED);
		EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, 0), 0, INTERFACE_STRING(CommonFormatPlainText));
		EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, LT_SYNTAX_HIGHLIGHTING_LANGUAGE_C), 0, "C/C++", -1);
		EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, LT_SYNTAX_HIGHLIGHTING_LANGUAGE_INI), 0, "Ini", -1);
		EsListViewFixedItemSelect(list, instance->syntaxHighlightingLanguage);

		list->messageUser = [] (EsElement *element, EsMessage *message) {
			if (message->type == LT_MSG_LIST_VIEW_SELECT) {
				Instance *instance = element->instance;
				EsGeneric newLanguage;

				if (EsListViewFixedItemGetSelected(((EsListView *) element), &newLanguage)) {
					SetLanguage(instance, newLanguage.u);
				}
			}

			return 0;
		};
	}

	EsMenuShow(menu);
}

void CommandZoom(Instance *instance, EsElement *, EsCommand *command) {
	int32_t delta = instance->scrollCumulative > 0 
		? instance->scrollCumulative / LT_SCROLL_WHEEL_NOTCH 
		: -(-instance->scrollCumulative / LT_SCROLL_WHEEL_NOTCH);
	instance->scrollCumulative -= delta * LT_SCROLL_WHEEL_NOTCH;

	if (command) delta += command->data.i;

	intptr_t presetSizeCount = sizeof(presetTextSizes) / sizeof(presetTextSizes[0]);
	int32_t newIndex = delta;

	for (intptr_t i = 0; i <= presetSizeCount; i++) {
		if (i == presetSizeCount || presetTextSizes[i] > instance->textSize) {
			newIndex = i - 1 + delta;
			break;
		}
	}

	if (newIndex < 0) newIndex = 0;
	if (newIndex > presetSizeCount - 1) newIndex = presetSizeCount - 1;

	if (instance->textSize != presetTextSizes[newIndex]) {
		globalTextSize = instance->textSize = presetTextSizes[newIndex];
		EsTextboxSetTextSize(instance->textboxDocument, instance->textSize);
	}
}

int TextboxDocumentMessage(EsElement *element, EsMessage *message) {
	if (message->type == LT_MSG_SCROLL_WHEEL && EsKeyboardIsCtrlHeld()) {
		// TODO Maybe detecting the input needed to do this (Ctrl+Scroll) should be dealt with in the API,
		// 	so that the user could potentially customize it.
		Instance *instance = element->instance;
		instance->scrollCumulative += message->scrollWheel.dy;
		CommandZoom(instance, element, nullptr);
		return LT_HANDLED;
	} else {
		return 0;
	}
}

bool StringEndsWith(const char *string, size_t stringBytes, const char *prefix, size_t prefixBytes, bool caseInsensitive) {
	string += stringBytes - 1;
	prefix += prefixBytes - 1;

	while (true) {
		if (!prefixBytes) return true;
		if (!stringBytes) return false;

		char c1 = *string;
		char c2 = *prefix;

		if (caseInsensitive) {
			if (c1 >= 'a' && c1 <= 'z') c1 = c1 - 'a' + 'A';
			if (c2 >= 'a' && c2 <= 'z') c2 = c2 - 'a' + 'A';
		}

		if (c1 != c2) return false;

		stringBytes--;
		prefixBytes--;
		string--;
		prefix--;
	}
}

int InstanceCallback(Instance *instance, EsMessage *message) {
	if (message->type == LT_MSG_INSTANCE_SAVE) {
		size_t byteCount;
		char *contents = EsTextboxGetContents(instance->textboxDocument, &byteCount);
		EsFileStoreWriteAll(message->instanceSave.file, contents, byteCount);

		bool fileNameContainsExtension = false;

		for (uintptr_t i = 0; i < (uintptr_t) message->instanceSave.nameOrPathBytes && i < 5; i++) {
			if (message->instanceSave.nameOrPath[message->instanceSave.nameOrPathBytes - i - 1] == '.') {
				fileNameContainsExtension = true;
			}
		}

		if (fileNameContainsExtension) {
			// Don't set a content type, it should be matched from the file extension.
		} else {
			EsUniqueIdentifier plainText = (EsUniqueIdentifier) 
				{{ 0x25, 0x65, 0x4C, 0x89, 0xE7, 0x29, 0xEA, 0x9E, 0xB5, 0xBE, 0xB5, 0xCA, 0xA7, 0x60, 0xBD, 0x3D }};
			EsFileStoreSetContentType(message->instanceSave.file, plainText);
		}

		EsHeapFree(contents);
		EsInstanceSaveComplete(instance, message->instanceSave.file, true);
	} else if (message->type == LT_MSG_INSTANCE_OPEN) {
		size_t fileSize;
		char *file = (char *) EsFileStoreReadAll(message->instanceOpen.file, &fileSize);

		if (!file) {
			EsInstanceOpenComplete(instance, message->instanceOpen.file, false);
		} else if (!EsUTF8IsValid(file, fileSize)) {
			EsInstanceOpenComplete(instance, message->instanceOpen.file, false);
		} else {
			EsTextboxSelectAll(instance->textboxDocument);
			EsTextboxInsert(instance->textboxDocument, file, fileSize);
			EsTextboxSetSelection(instance->textboxDocument, 0, 0, 0, 0);
			EsElementRelayout(instance->textboxDocument);

			if (StringEndsWith(message->instanceOpen.nameOrPath, message->instanceOpen.nameOrPathBytes, EsLiteral(".c"), true)
					|| StringEndsWith(message->instanceOpen.nameOrPath, message->instanceOpen.nameOrPathBytes, EsLiteral(".cpp"), true)
					|| StringEndsWith(message->instanceOpen.nameOrPath, message->instanceOpen.nameOrPathBytes, EsLiteral(".h"), true)) {
				SetLanguage(instance, LT_SYNTAX_HIGHLIGHTING_LANGUAGE_C);
			} else if (StringEndsWith(message->instanceOpen.nameOrPath, message->instanceOpen.nameOrPathBytes, EsLiteral(".ini"), true)) {
				SetLanguage(instance, LT_SYNTAX_HIGHLIGHTING_LANGUAGE_INI);
			} else {
				SetLanguage(instance, 0);
			}

			EsInstanceOpenComplete(instance, message->instanceOpen.file, true);
		}

		EsHeapFree(file);
	} else {
		return 0;
	}

	return LT_HANDLED;
}

void ProcessApplicationMessage(EsMessage *message) {
	if (message->type == LT_MSG_INSTANCE_CREATE) {
		Instance *instance = EsInstanceCreate(message, INTERFACE_STRING(TextEditorTitle));
		instance->callback = InstanceCallback;
		EsInstanceSetClassEditor(instance, &editorSettings);

		EsWindow *window = instance->window;
		EsWindowSetIcon(window, LT_ICON_ACCESSORIES_TEXT_EDITOR);
		EsButton *button;

		// Commands:

		uint32_t stableID = 1;

		EsCommandRegister(&instance->commandFindNext, instance, INTERFACE_STRING(CommonSearchNext), [] (Instance *instance, EsElement *, EsCommand *) {
			Find(instance, false);
		}, stableID++, "F3", true); 

		EsCommandRegister(&instance->commandFindPrevious, instance, INTERFACE_STRING(CommonSearchPrevious), [] (Instance *instance, EsElement *, EsCommand *) {
			Find(instance, true);
		}, stableID++, "Shift+F3", true); 

		EsCommandRegister(&instance->commandFind, instance, INTERFACE_STRING(CommonSearchOpen), [] (Instance *instance, EsElement *, EsCommand *) {
			EsWindowSwitchToolbar(instance->window, instance->toolbarSearch, LT_TRANSITION_ZOOM_OUT);
			EsElementFocus(instance->textboxSearch);
		}, stableID++, "Ctrl+F", true);

		EsCommandRegister(&instance->commandFormat, instance, INTERFACE_STRING(CommonFormatPopup), [] (Instance *instance, EsElement *, EsCommand *) {
			FormatPopupCreate(instance);
		}, stableID++, "Ctrl+Alt+T", true); 

		EsCommandRegister(&instance->commandZoomIn,  instance, INTERFACE_STRING(CommonZoomIn),  CommandZoom, stableID++, "Ctrl+=", true)->data.i =  1; 
		EsCommandRegister(&instance->commandZoomOut, instance, INTERFACE_STRING(CommonZoomOut), CommandZoom, stableID++, "Ctrl+-", true)->data.i = -1; 

		// Content:

		EsPanel *panel = EsPanelCreate(window, LT_CELL_FILL, LT_STYLE_PANEL_WINDOW_DIVIDER);
		uint64_t documentFlags = LT_CELL_FILL | LT_TEXTBOX_MULTILINE | LT_TEXTBOX_ALLOW_TABS | LT_TEXTBOX_MARGIN;
		instance->textboxDocument = EsTextboxCreate(panel, documentFlags, LT_STYLE_TEXTBOX_NO_BORDER);
		instance->textboxDocument->cName = "document";
		instance->textboxDocument->messageUser = TextboxDocumentMessage;
		instance->textSize = globalTextSize;
		EsTextboxSetTextSize(instance->textboxDocument, globalTextSize);
		EsTextboxSetUndoManager(instance->textboxDocument, instance->undoManager);
		EsElementFocus(instance->textboxDocument);

		// Main toolbar:

		EsElement *toolbarMain = instance->toolbarMain = EsWindowGetToolbar(window, true);

		EsFileMenuAddToToolbar(toolbarMain);
		
		button = EsButtonCreate(toolbarMain, LT_FLAGS_DEFAULT, {}, INTERFACE_STRING(CommonSearchOpen));
		button->accessKey = 'S';
		EsButtonSetIcon(button, LT_ICON_EDIT_FIND_SYMBOLIC);

		EsButtonOnCommand(button, [] (Instance *instance, EsElement *, EsCommand *) {
			EsWindowSwitchToolbar(instance->window, instance->toolbarSearch, LT_TRANSITION_SLIDE_UP);
			EsElementFocus(instance->textboxSearch);
		});

		EsSpacerCreate(toolbarMain, LT_CELL_H_FILL);

		button = EsButtonCreate(toolbarMain, LT_BUTTON_DROPDOWN, {}, INTERFACE_STRING(CommonFormatPopup));
		button->accessKey = 'M';
		EsButtonSetIcon(button, LT_ICON_FORMAT_TEXT_LARGER_SYMBOLIC);
		EsCommandAddButton(&instance->commandFormat, button);
		instance->buttonFormat = button;

		EsWindowSwitchToolbar(window, toolbarMain, LT_TRANSITION_NONE);

		// Search toolbar:

		EsElement *toolbarSearch = instance->toolbarSearch = EsWindowGetToolbar(window, true);

		button = EsButtonCreate(toolbarSearch, LT_FLAGS_DEFAULT, 0);
		button->cName = "go back", button->accessKey = 'X';
		EsButtonSetIcon(button, LT_ICON_GO_FIRST_SYMBOLIC);

		EsButtonOnCommand(button, [] (Instance *instance, EsElement *, EsCommand *) {
			EsWindowSwitchToolbar(instance->window, instance->toolbarMain, LT_TRANSITION_SLIDE_DOWN);
		});

		EsSpacerCreate(toolbarSearch, LT_FLAGS_DEFAULT, 0, 14, 0);

		EsTextDisplayCreate(toolbarSearch, LT_FLAGS_DEFAULT, 0, INTERFACE_STRING(CommonSearchPrompt));
		instance->textboxSearch = EsTextboxCreate(toolbarSearch, LT_FLAGS_DEFAULT, {});
		instance->textboxSearch->cName = "search textbox";
		instance->textboxSearch->accessKey = 'S';

		instance->textboxSearch->messageUser = [] (EsElement *element, EsMessage *message) {
			Instance *instance = element->instance;

			if (message->type == LT_MSG_KEY_DOWN && message->keyboard.scancode == LT_SCANCODE_ENTER) {
				EsCommand *command = (message->keyboard.modifiers & LT_MODIFIER_SHIFT) ? &instance->commandFindPrevious : &instance->commandFindNext;
				command->callback(instance, element, command);
				return LT_HANDLED;
			} else if (message->type == LT_MSG_KEY_DOWN && message->keyboard.scancode == LT_SCANCODE_ESCAPE) {
				EsWindowSwitchToolbar(instance->window, instance->toolbarMain, LT_TRANSITION_SLIDE_DOWN);
				EsElementFocus(instance->textboxDocument);
				return LT_HANDLED;
			} else if (message->type == LT_MSG_FOCUSED_START) {
				EsTextboxSelectAll(instance->textboxSearch);
			}

			return 0;
		};

		EsSpacerCreate(toolbarSearch, LT_FLAGS_DEFAULT, 0, 7, 0);
		instance->displaySearch = EsTextDisplayCreate(toolbarSearch, LT_CELL_H_FILL, {}, "");

		EsPanel *buttonGroup = EsPanelCreate(toolbarSearch, LT_PANEL_HORIZONTAL | LT_ELEMENT_AUTO_GROUP);
		button = EsButtonCreate(buttonGroup, LT_FLAGS_DEFAULT, {}, INTERFACE_STRING(CommonSearchNext));
		button->accessKey = 'N';
		EsCommandAddButton(&instance->commandFindNext, button);
		EsSpacerCreate(buttonGroup, LT_CELL_V_FILL, LT_STYLE_TOOLBAR_BUTTON_GROUP_SEPARATOR);
		button = EsButtonCreate(buttonGroup, LT_FLAGS_DEFAULT, {}, INTERFACE_STRING(CommonSearchPrevious));
		button->accessKey = 'P';
		EsCommandAddButton(&instance->commandFindPrevious, button);
	} else if (message->type == LT_MSG_APPLICATION_EXIT) {
		EsBuffer buffer = {};
		buffer.canGrow = true;
		EsBufferFormat(&buffer, "[general]\ntext_size=%d\n", globalTextSize);
		EsFileWriteAll(EsLiteral(SETTINGS_FILE), buffer.out, buffer.position);
		EsHeapFree(buffer.out);
	}
}

void _start() {
	_init();

	size_t settingsFileBytes = 0;
	char *settingsFile = (char *) EsFileReadAll(EsLiteral(SETTINGS_FILE), &settingsFileBytes);
	EsINIState state = { .buffer = settingsFile, .bytes = settingsFileBytes };

	while (EsINIParse(&state)) {
		if (0 == EsStringCompareRaw(state.section, state.sectionBytes, EsLiteral("general"))) {
			if (0 == EsStringCompareRaw(state.key, state.keyBytes, EsLiteral("text_size"))) {
				globalTextSize = EsIntegerParse(state.value, state.valueBytes);
			}
		}
	}

	EsHeapFree(settingsFile);

	while (true) {
		ProcessApplicationMessage(EsMessageReceive());
	}
}
