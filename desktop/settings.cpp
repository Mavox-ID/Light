struct SettingsInstance : EsInstance {
	EsPanel *switcher;
	EsPanel *mainPage;
	EsButton *undoButton, *backButton;
	Array<struct SettingsControl *> controls;
};

struct SettingsPage {
	const char *string;
	ptrdiff_t stringBytes;
	uint32_t iconID;
	void (*create)(EsElement *parent, SettingsPage *page);
	char accessKey;
};

struct SettingsControl {
#define SETTINGS_CONTROL_CHECKBOX (1)
#define SETTINGS_CONTROL_NUMBER (2)
#define SETTINGS_CONTROL_SLIDER (3)
#define SETTINGS_CONTROL_CHOICE_LIST (4)
	uint8_t type;
	bool originalValueBool;
	int32_t originalValueInt;
	EsGeneric originalValueData;
	int32_t minimumValue, maximumValue;
	uint32_t steps;
	double dragSpeed, dragValue, discreteStep;
	const char *suffix;
	size_t suffixBytes;
	const char *cConfigurationSection; 
	const char *cConfigurationKey; 
	EsElement *element;
};

struct WallpaperState {
	EsTextbox *textbox;
	EsButton *applyButton;
	char *fullPath;
	size_t fullPathBytes;
};

const EsStyle styleAllSettingsGroupContainer = {
	.metrics = {
		.mask = LT_THEME_METRICS_PREFERRED_WIDTH | LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_ALL,
		.insets = LT_RECT_1(5),
		.preferredWidth = 400,
		.gapMajor = 5,
		.gapMinor = 5,
	},
};

const EsStyle styleSettingsGroupContainer = {
};

const EsStyle styleSettingsGroupContainer2 = {
	.metrics = {
		.mask = LT_THEME_METRICS_PREFERRED_WIDTH | LT_THEME_METRICS_INSETS | LT_THEME_METRICS_GAP_MAJOR,
		.insets = LT_RECT_1(30),
		.preferredWidth = 400,
		.gapMajor = 15,
	},
};

const EsStyle styleWallpaperTextbox = {
    .inherit = LT_STYLE_TEXTBOX_BORDERED_SINGLE,
    .metrics = {
        .mask = LT_THEME_METRICS_PREFERRED_WIDTH,
        .preferredWidth = 150,
    },
};

const uint32_t windowColors[][8] = {
	{ 0xFF67EFC4, 0xCEF2E6, 0xFFA9E7D3, 0xFF94D1BD, 0xFF6BA996, 0xFFD0E2DD, 0xFFD1EAE2, 0xFFBAD1C9 },
	{ 0xFF54ACE5, 0xB1CEE1, 0xFF9BC3DD, 0xFF87AEC8, 0xFF6784A2, 0xFFC5D1D9, 0xFFC6D6E0, 0xFFB0BEC8 },
	{ 0xFF448CF5, 0x77B9F9, 0xFF9CBDED, 0xFF87A7D6, 0xFF5E7FB8, 0xFFD0DAE8, 0xFFD0DDF0, 0xFFB9C4D6 },
	{ 0xFF044CF5, 0x8FABEB, 0xFF7FA0ED, 0xFF6A8AD6, 0xFF4065B4, 0xFFC8D1E8, 0xFFC4D1F0, 0xFFAEBAD6 },
	{ 0xFFAC00FF, 0xD899F6, 0xFFD183F7, 0xFFB96DDF, 0xFF8F41B5, 0xFFE6D0F2, 0xFFEBCCFA, 0xFFD1B5DF },
	{ 0xFFFF0032, 0xF9A0B1, 0xFFF78399, 0xFFDF6D83, 0xFFC0436B, 0xFFF2D0D6, 0xFFFACCD5, 0xFFDFB5BD },
	{ 0xFFFF6042, 0xF9BBAF, 0xFFF7AEA1, 0xFFDF978A, 0xFFB5525F, 0xFFF2DCD8, 0xFFFADDD7, 0xFFDFC4BF },
	{ 0xFFFF7F24, 0xF5C5A3, 0xFFF7BC93, 0xFFDFA57D, 0xFFC7825F, 0xFFF2E0D4, 0xFFFAE2D2, 0xFFDFC9BA },
	{ 0xFFE8ECF9, 0xE5E7ED, 0xFFE9EBF1, 0xFFD2D3D9, 0xFFA9AAB0, 0xFFEAEAEC, 0xFFF1F1F4, 0xFFD6D7D9 },
	{ 0xFFB7BBC5, 0xA4A5AA, 0xFFB8BABE, 0xFF9A9BA0, 0xFF85878B, 0xFFE9E9EA, 0xFFDEDFE1, 0xFFB9BABC },
};

const EsStyle styleSettingsCheckboxGroup = {
	.metrics = {
		.mask = LT_THEME_METRICS_GAP_MAJOR | LT_THEME_METRICS_GAP_MINOR,
		.gapMajor = 0,
		.gapMinor = 0,
	},
};

const EsStyle styleSettingsOverlayPanel = {
	.metrics = {
		.mask = LT_THEME_METRICS_INSETS,
		.insets = LT_RECT_1(10),
	},
};

const EsStyle styleSettingsButton = {
	.inherit = LT_STYLE_PUSH_BUTTON_TOOLBAR,
	.metrics = {
		.mask = LT_THEME_METRICS_PREFERRED_WIDTH | LT_THEME_METRICS_PREFERRED_HEIGHT | LT_THEME_METRICS_LAYOUT_VERTICAL
			| LT_THEME_METRICS_GAP_ALL | LT_THEME_METRICS_TEXT_ALIGN | LT_THEME_METRICS_INSETS | LT_THEME_METRICS_ICON_SIZE,
		.insets = LT_RECT_2(0, 10),
		.preferredWidth = 0,
		.preferredHeight = 75,
		.gapMajor = 3,
		.gapMinor = 3,
		.gapWrap = 3,
		.textAlign = LT_TEXT_H_CENTER,
		.iconSize = 32,
		.layoutVertical = true,
	},
};

const EsStyle styleSliderRow = {
	.metrics = {
		.mask = LT_THEME_METRICS_GAP_MAJOR | LT_THEME_METRICS_INSETS,
		.insets = LT_RECT_4(0, 0, 3, 0),
		.gapMajor = 6,
	},
};

bool SettingsPutValue(const char *cConfigurationSection, const char *cConfigurationKey, char *newValue, size_t newValueBytes, 
		char **oldValue, size_t *oldValueBytes, bool duplicate, bool overwriteExisting) {
	if (duplicate) {
		char *newValueDuplicate = (char *) EsHeapAllocate(newValueBytes, false);

		if (!newValueDuplicate) {
			return false;
		}

		EsMemoryCopy(newValueDuplicate, newValue, newValueBytes);
		newValue = newValueDuplicate;
	}

	SystemConfigurationGroup *group = SystemConfigurationGetGroup(cConfigurationSection, -1, true);

	if (!group) {
		EsHeapFree(newValue);
		return false;
	}

	SystemConfigurationItem *item = SystemConfigurationGetItem(group, cConfigurationKey, -1, true);

	if (!item) {
		EsHeapFree(newValue);
		return false;
	}

	if (oldValue) {
		*oldValue = item->value;
	}

	if (oldValueBytes) {
		*oldValueBytes = item->valueBytes;
	}

	if (!overwriteExisting && item->valueBytes) {
		EsHeapFree(newValue);
	} else {
		if (!oldValue) {
			EsHeapFree(item->value);
		}

		item->value = newValue;
		item->valueBytes = newValueBytes;
	}

	return true;
}

void SettingsLoadDefaults() {
	SettingsPutValue("general", "click_chain_timeout_ms", EsLiteral("300"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "show_cursor_shadow", EsLiteral("1"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "scroll_lines_per_notch", EsLiteral("3"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "ui_scale", EsLiteral("100"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "window_color", EsLiteral("9"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "use_smart_quotes", EsLiteral("1"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "enable_hover_state", EsLiteral("1"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "enable_animations", EsLiteral("1"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "keyboard_layout", EsLiteral("us"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "save_with_file_extensions", EsLiteral("1"), nullptr, nullptr, true, false);
	SettingsPutValue("paths", "default_user_documents", EsLiteral("0:/"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "timezone_offset", EsLiteral("0"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "clock_12_hour", EsLiteral("0"), nullptr, nullptr, true, false);
	SettingsPutValue("general", "clock_show_seconds", EsLiteral("0"), nullptr, nullptr, true, false);
}

void SettingsUpdateGlobalAndWindowManager() {
	api.global->clickChainTimeoutMs = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("click_chain_timeout_ms"));
	api.global->swapLeftAndRightButtons = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("swap_left_and_right_buttons"));
	api.global->showCursorShadow = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("show_cursor_shadow"));
	api.global->useSmartQuotes = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("use_smart_quotes"));
	api.global->enableHoverState = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("enable_hover_state"));
	api.global->animationTimeMultiplier = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("enable_animations")) ? 1.0f : 0.0f;

	size_t keyboardLayoutBytes;
	char *keyboardLayout = EsSystemConfigurationReadString(EsLiteral("general"), EsLiteral("keyboard_layout"), &keyboardLayoutBytes);
	api.global->keyboardLayout = keyboardLayout && keyboardLayoutBytes >= 2 ? (keyboardLayout[0] | ((uint16_t) keyboardLayout[1] << 8)) : 1;
	EsHeapFree(keyboardLayout);

	{
		float newUIScale = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("ui_scale")) * 0.01f;
		bool changed = api.global->uiScale != newUIScale && api.global->uiScale;
		api.global->uiScale = newUIScale;

		if (changed) {
			EsMessage m;
			EsMemoryZero(&m, sizeof(EsMessage));
			m.type = LT_MSG_UI_SCALE_CHANGED;

			for (uintptr_t i = 0; i < desktop.allApplicationProcesses.Length(); i++) {
				EsMessagePostRemote(desktop.allApplicationProcesses[i]->handle, &m);
			}
		}
	}

	uint32_t cursorProperties = 0;
	if (EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("use_cursor_acceleration"))) cursorProperties |= CURSOR_USE_ACCELERATION;
	if (EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("use_cursor_alt_slow")))     cursorProperties |= CURSOR_USE_ALT_SLOW;
	int32_t cursorSpeed = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("cursor_speed"));
	double cursorSpeedFactor = EsCRTexp2(0.25 * cursorSpeed);
	cursorProperties |= (uint32_t) (cursorSpeedFactor * 0x100) << 16;
	cursorProperties |= (uint32_t) EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("cursor_trails")) << 13;
	EsSyscall(LT_SYSCALL_CURSOR_PROPERTILT_SET, cursorProperties, 0, 0, 0);
}

void SettingsBackButton(EsInstance *_instance, EsElement *, EsCommand *) {
	SettingsInstance *instance = (SettingsInstance *) _instance;

	EsElementSetDisabled(instance->undoButton, true);
	EsElementSetHidden(instance->undoButton, true);
	EsElementSetDisabled(instance->backButton, true);

	instance->controls.Free();

	EsPanelSwitchTo(instance->switcher, instance->mainPage, LT_TRANSITION_ZOOM_OUT, LT_PANEL_SWITCHER_DESTROY_PREVIOUS_AFTER_TRANSITION, 1.0f);
	ConfigurationWriteToFile();
}

void SettingsNumberBoxSetValue(EsElement *element, double newValueDouble) {
	EsTextbox *textbox = (EsTextbox *) element;
	SettingsInstance *instance = (SettingsInstance *) textbox->instance;
	SettingsControl *control = (SettingsControl *) textbox->userData.p;

	int32_t newValue = (int32_t) (newValueDouble / control->discreteStep + 0.5) * control->discreteStep;

	newValue = ClampInteger(control->minimumValue, control->maximumValue, newValue);
	char buffer[64];
	size_t bytes = EsStringFormat(buffer, sizeof(buffer), "%i%s", newValue, control->suffixBytes, control->suffix);
	EsTextboxSelectAll(textbox);
	EsTextboxInsert(textbox, buffer, bytes);

	EsMutexAcquire(&api.systemConfigurationMutex);

	char *value = (char *) EsHeapAllocate(65, true), *_oldValue;
	size_t valueBytes = EsStringFormat(value, 64, "%fd", LT_STRING_FORMAT_SIMPLE, newValue), _oldValueBytes;
	int32_t oldValue = 0;

	if (SettingsPutValue(control->cConfigurationSection, control->cConfigurationKey, value, valueBytes, 
			&_oldValue, &_oldValueBytes, false, true)) {
		oldValue = EsIntegerParse(_oldValue, _oldValueBytes);
		EsHeapFree(_oldValue);
	}

	EsMutexRelease(&api.systemConfigurationMutex);

	if (oldValue != newValue) {
		SettingsUpdateGlobalAndWindowManager();
		EsElementSetDisabled(instance->undoButton, false);
		desktop.configurationModified = true;
	}
}

void SettingsUndoButton(EsInstance *_instance, EsElement *, EsCommand *) {
	SettingsInstance *instance = (SettingsInstance *) _instance;

	for (uintptr_t i = 0; i < instance->controls.Length(); i++) {
		SettingsControl *control = instance->controls[i];

		if (control->type == SETTINGS_CONTROL_CHECKBOX) {
			EsButtonSetCheck((EsButton *) control->element, control->originalValueBool ? LT_CHECK_CHECKED : LT_CHECK_UNCHECKED, true);
		} else if (control->type == SETTINGS_CONTROL_NUMBER) {
			SettingsNumberBoxSetValue(control->element, control->originalValueInt);
		} else if (control->type == SETTINGS_CONTROL_SLIDER) {
			EsSliderSetValue((EsSlider *) control->element, LinearMap(control->minimumValue, control->maximumValue, 0, 1, control->originalValueInt), true);
		} else if (control->type == SETTINGS_CONTROL_CHOICE_LIST) {
			EsListViewFixedItemSelect((EsListView *) control->element, control->originalValueData);
		}
	}

	EsElementSetDisabled(instance->undoButton, true);
}

void SettingsAddTitle(EsElement *container, SettingsPage *page) {
	EsPanel *row = EsPanelCreate(container, LT_CELL_H_FILL | LT_PANEL_HORIZONTAL);
	EsIconDisplayCreate(row, LT_FLAGS_DEFAULT, LT_STYLE_ICON_DISPLAY, page->iconID);
	EsSpacerCreate(row, LT_FLAGS_DEFAULT, 0, 10, 0);
	EsTextDisplayCreate(row, LT_CELL_H_FILL, LT_STYLE_TEXT_HEADING2, page->string, page->stringBytes);
	EsSpacerCreate(container, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);
}

void SettingsPageUnimplemented(EsElement *element, SettingsPage *page) {
	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL | LT_CELL_H_SHRINK, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *warningRow = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsIconDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, LT_ICON_DIALOG_WARNING);
	EsTextDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, "Work in progress" ELLIPSIS);
}

void SettingsCheckboxCommand(EsInstance *_instance, EsElement *element, EsCommand *) {
	EsButton *button = (EsButton *) element;
	SettingsInstance *instance = (SettingsInstance *) _instance;
	SettingsControl *control = (SettingsControl *) button->userData.p;
	bool newValue = EsButtonGetCheck(button) == LT_CHECK_CHECKED;

	EsMutexAcquire(&api.systemConfigurationMutex);

	char *value = (char *) EsHeapAllocate(2, true), *_oldValue;
	size_t _oldValueBytes;
	value[0] = newValue ? '1' : '0';
	bool oldValue = false;

	if (SettingsPutValue(control->cConfigurationSection, control->cConfigurationKey, value, 1, 
			&_oldValue, &_oldValueBytes, false, true)) {
		oldValue = EsIntegerParse(_oldValue, _oldValueBytes);
		EsHeapFree(_oldValue);
	}

	EsMutexRelease(&api.systemConfigurationMutex);

	if (oldValue == newValue) return;
	SettingsUpdateGlobalAndWindowManager();
	EsElementSetDisabled(instance->undoButton, false);
	desktop.configurationModified = true;
}

void SettingsAddCheckbox(EsElement *table, const char *string, ptrdiff_t stringBytes, char accessKey,
		const char *cConfigurationSection, const char *cConfigurationKey) {
	SettingsInstance *instance = (SettingsInstance *) table->instance;

	SettingsControl *control = (SettingsControl *) EsHeapAllocate(sizeof(SettingsControl), true);
	control->type = SETTINGS_CONTROL_CHECKBOX;
	control->cConfigurationSection = cConfigurationSection;
	control->cConfigurationKey = cConfigurationKey;
	control->originalValueBool = EsSystemConfigurationReadInteger(control->cConfigurationSection, -1, control->cConfigurationKey, -1);

	EsButton *button = EsButtonCreate(table, LT_CELL_H_FILL | LT_BUTTON_CHECKBOX | LT_ELEMENT_FREE_USER_DATA | LT_ELEMENT_STICKY_ACCESS_KEY, 0, string, stringBytes);
	button->userData = control;
	button->accessKey = accessKey;
	if (control->originalValueBool) EsButtonSetCheck(button, LT_CHECK_CHECKED, false);
	EsButtonOnCommand(button, SettingsCheckboxCommand);

	control->element = button;
	instance->controls.Add(control);
}

int SettingsNumberBoxMessage(EsElement *element, EsMessage *message) {
	EsTextbox *textbox = (EsTextbox *) element;
	SettingsControl *control = (SettingsControl *) textbox->userData.p;

	if (message->type == LT_MSG_TEXTBOX_EDIT_END || message->type == LT_MSG_TEXTBOX_NUMBER_DRAG_END) {
		size_t bytes;
		char *expression = EsTextboxGetContents(textbox, &bytes);

		if (control->suffixBytes && bytes > control->suffixBytes && 0 == EsMemoryCompare(control->suffix, 
					expression + bytes - control->suffixBytes, control->suffixBytes)) {
			expression[bytes - control->suffixBytes] = 0;
		}

		EsCalculationValue value = EsCalculateFromUserExpression(expression); 
		EsHeapFree(expression);

		if (!value.error) {
			SettingsNumberBoxSetValue(element, value.number);
			return LT_HANDLED;
		} else {
			return LT_REJECTED;
		}
	} else if (message->type == LT_MSG_TEXTBOX_NUMBER_DRAG_START) {
		control->dragValue = EsTextboxGetContentsAsDouble(textbox);
	} else if (message->type == LT_MSG_TEXTBOX_NUMBER_DRAG_DELTA) {
		double newValue = ClampDouble(control->minimumValue, control->maximumValue, 
				control->dragValue + message->numberDragDelta.delta * (message->numberDragDelta.fast ? 10.0 : 1.0) * control->dragSpeed);
		control->dragValue = newValue;
		char buffer[64];
		size_t bytes = EsStringFormat(buffer, sizeof(buffer), "%i%s", (int32_t) newValue, control->suffixBytes, control->suffix);
		EsTextboxSelectAll(textbox);
		EsTextboxInsert(textbox, buffer, bytes);
		return LT_HANDLED;
	}

	return 0;
}


void SettingsAddNumberBox(EsElement *table, const char *string, ptrdiff_t stringBytes, char accessKey, 
		const char *cConfigurationSection, const char *cConfigurationKey,
		int32_t minimumValue, int32_t maximumValue, const char *suffix, ptrdiff_t suffixBytes, double dragSpeed, double discreteStep) {
	if (suffixBytes == -1) {
		suffixBytes = EsCStringLength(suffix);
	}

	SettingsInstance *instance = (SettingsInstance *) table->instance;

	SettingsControl *control = (SettingsControl *) EsHeapAllocate(sizeof(SettingsControl), true);
	control->type = SETTINGS_CONTROL_NUMBER;
	control->cConfigurationSection = cConfigurationSection;
	control->cConfigurationKey = cConfigurationKey;
	control->originalValueInt = EsSystemConfigurationReadInteger(control->cConfigurationSection, -1, control->cConfigurationKey, -1);
	control->suffix = suffix;
	control->suffixBytes = suffixBytes;
	control->minimumValue = minimumValue;
	control->maximumValue = maximumValue;
	control->dragSpeed = dragSpeed;
	control->discreteStep = discreteStep;

	EsTextDisplayCreate(table, LT_CELL_H_RIGHT, 0, string, stringBytes); 
	EsTextbox *textbox = EsTextboxCreate(table, LT_CELL_H_LEFT | LT_TEXTBOX_EDIT_BASED | LT_ELEMENT_FREE_USER_DATA, LT_STYLE_TEXTBOX_BORDERED_SINGLE_MEDIUM);
	EsTextboxUseNumberOverlay(textbox, false);
	textbox->userData = control;
	textbox->accessKey = accessKey;
	textbox->messageUser = SettingsNumberBoxMessage;

	char buffer[64];
	size_t bytes = EsStringFormat(buffer, sizeof(buffer), "%i%s", control->originalValueInt, suffixBytes, suffix);
	EsTextboxInsert(textbox, buffer, bytes);

	control->element = textbox;
	instance->controls.Add(control);
}

int SettingsSliderMessage(EsElement *element, EsMessage *message) {
	EsSlider *slider = (EsSlider *) element;
	SettingsInstance *instance = (SettingsInstance *) slider->instance;
	SettingsControl *control = (SettingsControl *) slider->userData.p;

	if (message->type == LT_MSG_SLIDER_MOVED && !message->sliderMoved.inDrag) {
		EsMutexAcquire(&api.systemConfigurationMutex);

		int32_t newValue = LinearMap(0, 1, control->minimumValue, control->maximumValue, EsSliderGetValue(slider));
		char *value = (char *) EsHeapAllocate(65, true), *_oldValue;
		size_t valueBytes = EsStringFormat(value, 64, "%fd", LT_STRING_FORMAT_SIMPLE, newValue), _oldValueBytes;
		int32_t oldValue = 0;

		if (SettingsPutValue(control->cConfigurationSection, control->cConfigurationKey, value, valueBytes, 
					&_oldValue, &_oldValueBytes, false, true)) {
			oldValue = EsIntegerParse(_oldValue, _oldValueBytes);
			EsHeapFree(_oldValue);
		}

		EsMutexRelease(&api.systemConfigurationMutex);

		if (oldValue != newValue) {
			SettingsUpdateGlobalAndWindowManager();
			EsElementSetDisabled(instance->undoButton, false);
			desktop.configurationModified = true;
		}
	}

	return 0;
}

void SettingsAddSlider(EsElement *table, const char *string, ptrdiff_t stringBytes, char accessKey, 
		const char *cConfigurationSection, const char *cConfigurationKey,
		int32_t minimumValue, int32_t maximumValue, uint32_t steps,
		const char *lowString, ptrdiff_t lowStringBytes, const char *highString, ptrdiff_t highStringBytes) {
	SettingsInstance *instance = (SettingsInstance *) table->instance;

	SettingsControl *control = (SettingsControl *) EsHeapAllocate(sizeof(SettingsControl), true);
	control->type = SETTINGS_CONTROL_SLIDER;
	control->cConfigurationSection = cConfigurationSection;
	control->cConfigurationKey = cConfigurationKey;
	control->originalValueInt = EsSystemConfigurationReadInteger(control->cConfigurationSection, -1, control->cConfigurationKey, -1);
	control->minimumValue = minimumValue;
	control->maximumValue = maximumValue;
	control->steps = steps;

	EsPanel *stack = EsPanelCreate(table, LT_CELL_H_FILL);
	EsTextDisplayCreate(stack, LT_CELL_H_LEFT, 0, string, stringBytes); 
	EsPanel *row = EsPanelCreate(stack, LT_PANEL_HORIZONTAL | LT_CELL_H_FILL, EsStyleIntern(&styleSliderRow));
	EsTextDisplayCreate(row, LT_CELL_H_PUSH | LT_CELL_H_RIGHT, 0, lowString, lowStringBytes); 
	EsSlider *slider = EsSliderCreate(row, LT_ELEMENT_FREE_USER_DATA, 0, 
			LinearMap(control->minimumValue, control->maximumValue, 0, 1, control->originalValueInt), steps);
	EsTextDisplayCreate(row, LT_CELL_H_PUSH | LT_CELL_H_LEFT, 0, highString, highStringBytes); 
	slider->userData = control;
	slider->accessKey = accessKey;
	slider->messageUser = SettingsSliderMessage;

	control->element = slider;
	instance->controls.Add(control);
}

int SettingsChoiceListMessage(EsElement *element, EsMessage *message) {
	EsListView *list = (EsListView *) element;
	SettingsInstance *instance = (SettingsInstance *) list->instance;
	SettingsControl *control = (SettingsControl *) list->userData.p;

	if (message->type == LT_MSG_LIST_VIEW_SELECT) {
		EsGeneric _newValue;

		if (EsListViewFixedItemGetSelected(((EsListView *) element), &_newValue)) {
			EsMutexAcquire(&api.systemConfigurationMutex);

			const char *newValue = (const char *) _newValue.p;
			size_t newValueBytes = 0;
			while (newValue[newValueBytes] && newValue[newValueBytes] != '=') newValueBytes++;

			char *value = (char *) EsHeapAllocate(newValueBytes, false), *_oldValue;
			EsMemoryCopy(value, newValue, newValueBytes);
			size_t _oldValueBytes;
			bool changed = true;

			if (SettingsPutValue(control->cConfigurationSection, control->cConfigurationKey, value, newValueBytes, 
						&_oldValue, &_oldValueBytes, false, true)) {
				changed = _oldValueBytes != newValueBytes || EsMemoryCompare(_oldValue, newValue, newValueBytes);
				EsHeapFree(_oldValue);
			}

			EsMutexRelease(&api.systemConfigurationMutex);

			if (changed) {
				SettingsUpdateGlobalAndWindowManager();
				EsElementSetDisabled(instance->undoButton, false);
				desktop.configurationModified = true;
			}
		}
	}

	return 0;
}

EsListView *SettingsAddChoiceList(EsElement *table, const char *string, ptrdiff_t stringBytes, char accessKey, 
		const char *cConfigurationSection, const char *cConfigurationKey) {
	SettingsInstance *instance = (SettingsInstance *) table->instance;

	SettingsControl *control = (SettingsControl *) EsHeapAllocate(sizeof(SettingsControl), true);
	control->type = SETTINGS_CONTROL_CHOICE_LIST;
	control->cConfigurationSection = cConfigurationSection;
	control->cConfigurationKey = cConfigurationKey;

	EsTextDisplayCreate(table, LT_CELL_H_RIGHT | LT_CELL_V_TOP, 0, string, stringBytes);
	EsListView *list = EsListViewCreate(table, LT_CELL_H_FILL | LT_LIST_VIEW_CHOICE_SELECT | LT_LIST_VIEW_FIXED_ITEMS, LT_STYLE_LIST_CHOICE_BORDERED);
	list->accessKey = accessKey;
	list->userData = control;
	list->messageUser = SettingsChoiceListMessage;

	control->element = list;
	instance->controls.Add(control);
	return list;
}

int SettingsDoubleClickTestMessage(EsElement *element, EsMessage *message) {
	if (message->type == LT_MSG_MOUSE_LEFT_DOWN) {
		if (message->mouseDown.clickChainCount >= 2) {
			element->customStyleState ^= THEME_STATE_CHECKED;
			EsElementRepaint(element);
		}

		return LT_HANDLED;
	}

	return 0;
}

void SettingsPageMouse(EsElement *element, SettingsPage *page) {
	EsElementSetHidden(((SettingsInstance *) element->instance)->undoButton, false);

	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *table;

	SettingsAddSlider(container, INTERFACE_STRING(DesktopSettingsMouseSpeed), 'M', "general", "cursor_speed", -20, 20, 41, 
			INTERFACE_STRING(DesktopSettingsMouseSpeedSlow), INTERFACE_STRING(DesktopSettingsMouseSpeedFast));

	table = EsPanelCreate(container, LT_CELL_H_FILL, EsStyleIntern(&styleSettingsCheckboxGroup));
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsMouseUseAcceleration), 'C', "general", "use_cursor_acceleration");
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsMouseSlowOnAlt), 'O', "general", "use_cursor_alt_slow");

	EsSpacerCreate(container, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);

	SettingsAddSlider(container, INTERFACE_STRING(DesktopSettingsMouseCursorTrails), 'T', "general", "cursor_trails", 0, 7, 8, 
			INTERFACE_STRING(DesktopSettingsMouseCursorTrailsNone), INTERFACE_STRING(DesktopSettingsMouseCursorTrailsMany));

	table = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	SettingsAddNumberBox(table, INTERFACE_STRING(DesktopSettingsMouseLinesPerScrollNotch), 'S', "general", "scroll_lines_per_notch", 
			1, 100, nullptr, 0, 0.04, 1);

	table = EsPanelCreate(container, LT_CELL_H_FILL, EsStyleIntern(&styleSettingsCheckboxGroup));
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsMouseSwapLeftAndRightButtons), 'B', "general", "swap_left_and_right_buttons");
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsMouseShowShadow), 'W', "general", "show_cursor_shadow");
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsMouseLocateCursorOnCtrl), 'L', "general", "locate_cursor_on_ctrl");

	EsSpacerCreate(container, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);

	table = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	SettingsAddNumberBox(table, INTERFACE_STRING(DesktopSettingsMouseDoubleClickSpeed), 'D', "general", "click_chain_timeout_ms", 
			100, 1500, INTERFACE_STRING(CommonUnitMilliseconds), 1.0, 1);

	EsPanel *testBox = EsPanelCreate(container, LT_CELL_H_FILL);
	EsTextDisplayCreate(testBox, LT_CELL_H_FILL, LT_STYLE_TEXT_PARAGRAPH, INTERFACE_STRING(DesktopSettingsMouseTestDoubleClickIntroduction));
	EsSpacerCreate(testBox, LT_FLAGS_DEFAULT, 0, 0, 5);
	EsCustomElementCreate(testBox, LT_FLAGS_DEFAULT, LT_STYLE_DOUBLE_CLICK_TEST)->messageUser = SettingsDoubleClickTestMessage;
}

void SettingsPageTime(EsElement *element, SettingsPage *page) {
	EsElementSetHidden(((SettingsInstance *) element->instance)->undoButton, false);

	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *table = EsPanelCreate(container, LT_CELL_H_FILL | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsPanelSetBands(table, 2);

	SettingsAddNumberBox(table, "Timezone Offset (UTC)", 21, 'Z', "general", "timezone_offset", 
			-12, 12, " h", 2, 0.1, 1);

	EsSpacerCreate(container, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);

	EsPanel *checkboxTable = EsPanelCreate(container, LT_CELL_H_FILL, EsStyleIntern(&styleSettingsCheckboxGroup));
	SettingsAddCheckbox(checkboxTable, "12-hour format (AM/PM)", 22, 'A', "general", "clock_12_hour");
	SettingsAddCheckbox(checkboxTable, "Show seconds", 12, 'S', "general", "clock_show_seconds");
}

void SettingsPageKeyboard(EsElement *element, SettingsPage *page) {
	EsElementSetHidden(((SettingsInstance *) element->instance)->undoButton, false);

	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *table;
	EsTextbox *textbox;

	table = EsPanelCreate(container, LT_CELL_H_FILL | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsPanelSetBands(table, 2);

	EsListView *list = SettingsAddChoiceList(table, INTERFACE_STRING(DesktopSettingsKeyboardLayout), 'L', "general", "keyboard_layout");

	EsINIState s = {};
	s.buffer = (char *) EsBundleFind(&bundleDesktop, EsLiteral("Keyboard Layouts.ini"), &s.bytes);
	void *activeKeyboardLayout = nullptr;

	while (EsINIParse(&s)) {
		if (s.key[0] != ';') {
			EsListViewFixedItemSetString(list, EsListViewFixedItemInsert(list, s.key), 0, s.value, s.valueBytes);
			EsAssert(s.keyBytes == 2);

			if (s.key[0] + ((uint16_t) s.key[1] << 8) == api.global->keyboardLayout) {
				activeKeyboardLayout = s.key;
				((SettingsControl *) list->userData.p)->originalValueData = activeKeyboardLayout;
			}
		}
	}

	EsListViewFixedItemSelect(list, activeKeyboardLayout);

	table = EsPanelCreate(container, LT_CELL_H_FILL, EsStyleIntern(&styleSettingsCheckboxGroup));
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsKeyboardUseSmartQuotes), 'Q', "general", "use_smart_quotes");

	EsSpacerCreate(container, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);

	EsPanel *warningRow = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsIconDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, LT_ICON_DIALOG_WARNING);
	EsTextDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, "Work in progress" ELLIPSIS);

	table = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsPanelSetBands(table, 2);

	EsTextDisplayCreate(table, LT_CELL_H_RIGHT, 0, INTERFACE_STRING(DesktopSettingsKeyboardKeyRepeatDelay));
	textbox = EsTextboxCreate(table, LT_CELL_H_LEFT | LT_TEXTBOX_EDIT_BASED, LT_STYLE_TEXTBOX_BORDERED_SINGLE_MEDIUM);
	textbox->accessKey = 'D';
	EsTextboxUseNumberOverlay(textbox, false);
	EsTextboxInsert(textbox, "400 ms");

	EsTextDisplayCreate(table, LT_CELL_H_RIGHT, 0, INTERFACE_STRING(DesktopSettingsKeyboardKeyRepeatRate));
	textbox = EsTextboxCreate(table, LT_CELL_H_LEFT | LT_TEXTBOX_EDIT_BASED, LT_STYLE_TEXTBOX_BORDERED_SINGLE_MEDIUM);
	textbox->accessKey = 'R';
	EsTextboxUseNumberOverlay(textbox, false);
	EsTextboxInsert(textbox, "400 ms");

	EsTextDisplayCreate(table, LT_CELL_H_RIGHT, 0, INTERFACE_STRING(DesktopSettingsKeyboardCaretBlinkRate));
	textbox = EsTextboxCreate(table, LT_CELL_H_LEFT | LT_TEXTBOX_EDIT_BASED, LT_STYLE_TEXTBOX_BORDERED_SINGLE_MEDIUM);
	textbox->accessKey = 'B';
	EsTextboxUseNumberOverlay(textbox, false);
	EsTextboxInsert(textbox, "500 ms");
}

void SettingsPageDisplay(EsElement *element, SettingsPage *page) {
	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *warningRow = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsIconDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, LT_ICON_DIALOG_WARNING);
	EsTextDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, "Work in progress" ELLIPSIS);

	EsPanel *table = EsPanelCreate(container, LT_CELL_H_FILL | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsPanelSetBands(table, 2);
	SettingsAddNumberBox(table, INTERFACE_STRING(DesktopSettingsDisplayUIScale), 'S', "general", "ui_scale", 
			100, 400, INTERFACE_STRING(CommonUnitPercent), 0.05, 5);
}

void SettingsPageFiles(EsElement *element, SettingsPage *page) {
	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *table = EsPanelCreate(container, LT_CELL_H_FILL, EsStyleIntern(&styleSettingsCheckboxGroup));
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsFilesSaveWithFileExtension), 'X', "general", "save_with_file_extensions");
}

int SettingsColorButtonMessage(EsElement *element, EsMessage *message) {
	if (message->type == LT_MSG_PAINT_BACKGROUND) {
		EsRectangle bounds = EsPainterBoundsClient(message->painter);
		bounds.r = bounds.l + 14 * EsElementGetScaleFactor(element);
		bounds = EsRectangleFit(bounds, LT_RECT_2S(1, 1), true);
		uint32_t cornerRadius = Width(bounds);
		EsCornerRadii cornerRadii = { cornerRadius, cornerRadius, cornerRadius, cornerRadius };
		int32_t borderSize = 1 * EsElementGetScaleFactor(element);

		uint32_t color = EsColorBlend(windowColors[element->userData.u][0], 0x20FFFFFF, false);
		if (element->window->pressed == element) color = EsColorBlend(color, 0x40000000, false);
		EsDrawRoundedRectangle(message->painter, bounds, color, EsColorBlend(color, 0x40000000, false), LT_RECT_1(borderSize), cornerRadii);

		if (EsButtonGetCheck((EsButton *) element) == LT_CHECK_CHECKED) {
			int32_t inset = 4 * EsElementGetScaleFactor(element);
			uint32_t bulletColor = EsColorIsLight(color) ? 0xFF000000 : 0xFFFFFFFF;
			EsDrawRoundedRectangle(message->painter, EsRectangleAdd(bounds, LT_RECT_1I(inset)), bulletColor, 0, LT_RECT_1(0), cornerRadii);
		}

		return LT_HANDLED;
	}

	return 0;
}

int SettingsTransitionWindowMessage(EsElement *element, EsMessage *message) {
	EsWindow *window = element->window;

	if (message->type == LT_MSG_ANIMATE) {
		window->animationTime += message->animate.deltaMs;
		double progress = window->animationTime / GetConstantNumber("settingsTransitionDuration");

		if (progress > 1) {
			EsElementDestroy(window);
		} else {
			EsSyscall(LT_SYSCALL_WINDOW_SET_PROPERTY, window->handle, 0xFF * (1 - progress), 0, LT_WINDOW_PROPERTY_ALPHA);
			EsSyscall(LT_SYSCALL_SCREEN_FORCE_UPDATE, true, 0, 0, 0);
			message->animate.complete = false;
			message->animate.waitMs = 10;
			return LT_HANDLED;
		}
	}

	return 0;
}

void SettingsCreateTransitionWindow() {
	if (!desktop.setupDesktopUIComplete) {
		return;
	}

	EsRectangle screen;
	EsSyscall(LT_SYSCALL_SCREEN_BOUNDS_GET, 0, (uintptr_t) &screen, 0, 0);
	EsRectangle region = LT_RECT_2S(LT_RECT_WIDTH(screen), LT_RECT_HEIGHT(screen));

	uint32_t *screenshot = (uint32_t *) EsHeapAllocate(region.r * region.b * 4, false);

	if (!screenshot) {
		return;
	}

	EsSyscall(LT_SYSCALL_SCREEN_GET_BITS, 0, (uintptr_t) &region, (uintptr_t) screenshot, 0);

	for (intptr_t i = 0; i < region.r * region.b; i++) {
		screenshot[i] |= 0xFF000000;
	}

	EsWindow *transition = EsWindowCreate(nullptr, LT_WINDOW_PLAIN);

	if (transition) {
		transition->messageUser = SettingsTransitionWindowMessage;
		transition->doNotPaint = true;
		transition->StartAnimating();
		EsSyscall(LT_SYSCALL_WINDOW_MOVE, transition->handle, (uintptr_t) &screen, 0, LT_WINDOW_MOVE_ALWAYS_ON_TOP);
		EsSyscall(LT_SYSCALL_WINDOW_SET_PROPERTY, transition->handle, 0, 0, LT_WINDOW_PROPERTY_SOLID);
		EsSyscall(LT_SYSCALL_WINDOW_SET_BITS, transition->handle, (uintptr_t) &region, (uintptr_t) screenshot, 0);
		EsSyscall(LT_SYSCALL_SCREEN_FORCE_UPDATE, true, 0, 0, 0);
	}

	EsHeapFree(screenshot);
}

void SettingsWindowColorUpdated() {
    SettingsCreateTransitionWindow();

    size_t bytes = 0;
    char *colorStr = EsSystemConfigurationReadString(EsLiteral("general"), EsLiteral("window_color"), &bytes);
    
    if (colorStr) {
        bool isRGB = false;
        for (size_t i = 0; i < bytes; i++) {
            if (colorStr[i] == ',') {
                isRGB = true;
                break;
            }
        }

        if (isRGB) {
            int r = 0, g = 0, b = 0;
            char *ptr = colorStr;
            size_t remaining = bytes;

            r = EsIntegerParse(ptr, remaining);
            while (remaining > 0 && *ptr != ',') { ptr++; remaining--; } 
            if (remaining > 0 && *ptr == ',') { ptr++; remaining--; }
            
            g = EsIntegerParse(ptr, remaining);
            while (remaining > 0 && *ptr != ',') { ptr++; remaining--; } 
            if (remaining > 0 && *ptr == ',') { ptr++; remaining--; }
            
            b = EsIntegerParse(ptr, remaining);

            uint32_t rgb = (r << 16) | (g << 8) | b;
            for (int i = 0; i < 6; i++) theming.windowColors[i] = rgb;
            desktop.wallpaperBackgroundColor = rgb;
        } else {
            int index = EsIntegerParse(colorStr, bytes);
            if (index >= 0 && index < (int)(sizeof(windowColors) / sizeof(windowColors[0]))) {
                EsMemoryCopy(theming.windowColors, &windowColors[index][2], sizeof(theming.windowColors));
                desktop.wallpaperBackgroundColor = windowColors[index][1];
            }
        }
        EsHeapFree(colorStr);
    }

    for (uintptr_t i = 0; i < gui.allWindows.Length(); i++) {
        if (gui.allWindows[i]->windowStyle == LT_WINDOW_CONTAINER) {
            gui.allWindows[i]->Repaint(true);
            UIWindowNeedsUpdate(gui.allWindows[i]);
        }
    }

    if (desktop.isWallpaperBackgroundColorVisible) {
        EsThreadInformation threadInfo = {};
        EsThreadCreate(WallpaperLoad, &threadInfo, 0);
    }
}

void SettingsColorButtonCommand(EsInstance *, EsElement *element, EsCommand *) {
    if (EsButtonGetCheck((EsButton *) element) != LT_CHECK_CHECKED) {
        return;
    }

    int colorIndex = (int)element->userData.u;
    char buffer[64];
    size_t bytes = EsStringFormat(buffer, sizeof(buffer), "%d", colorIndex);
    
    SettingsPutValue("general", "window_color", buffer, bytes, nullptr, nullptr, true, true);

    SettingsWindowColorUpdated();
    
    desktop.configurationModified = true;
}


void SettingsPageTheme(EsElement *element, SettingsPage *page) {
	EsPanel *content = EsPanelCreate(element, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO | LT_PANEL_H_SCROLL_AUTO, EsStyleIntern(&styleSettingsGroupContainer));
	EsPanel *container = EsPanelCreate(content, LT_PANEL_VERTICAL, EsStyleIntern(&styleSettingsGroupContainer2));
	SettingsAddTitle(container, page);

	EsPanel *warningRow = EsPanelCreate(container, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsIconDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, LT_ICON_DIALOG_WARNING);
	EsTextDisplayCreate(warningRow, LT_FLAGS_DEFAULT, 0, "Work in progress" ELLIPSIS);

	EsPanel *table = EsPanelCreate(container, LT_CELL_H_FILL | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL, LT_STYLE_PANEL_FORM_TABLE);
	EsPanelSetBands(table, 2);

	EsTextDisplayCreate(table, LT_CELL_H_LEFT, 0, INTERFACE_STRING(DesktopSettingsThemeWallpaper));

        EsPanel *wallpaperRow = EsPanelCreate(table, LT_CELL_H_FILL | LT_CELL_V_CENTER | LT_PANEL_HORIZONTAL);
	
	WallpaperState *wpState = (WallpaperState *) EsHeapAllocate(sizeof(WallpaperState), true);
	
	size_t currentWallpaperBytes;
	char *currentWallpaper = EsSystemConfigurationReadString(EsLiteral("general"), EsLiteral("wallpaper"), &currentWallpaperBytes);
	
	if (currentWallpaper && currentWallpaperBytes > 0) {
		wpState->fullPath = (char *) EsHeapAllocate(currentWallpaperBytes, false);
		EsMemoryCopy(wpState->fullPath, currentWallpaper, currentWallpaperBytes);
		wpState->fullPathBytes = currentWallpaperBytes;
	}
	EsTextbox *textbox = EsTextboxCreate(wallpaperRow, LT_CELL_H_FILL | LT_TEXTBOX_EDIT_BASED | LT_ELEMENT_FREE_USER_DATA, EsStyleIntern(&styleWallpaperTextbox));
	wpState->textbox = textbox;
	textbox->userData.p = wpState;
	
	if (currentWallpaper && currentWallpaperBytes > 0) {
		EsTextboxInsert(textbox, currentWallpaper, currentWallpaperBytes);
	}
	EsHeapFree(currentWallpaper);
	
	EsButton *applyButton = EsButtonCreate(wallpaperRow, LT_BUTTON_DEFAULT, 0, "Apply");
	wpState->applyButton = applyButton;
	EsElementSetDisabled(applyButton, true);
	applyButton->userData.p = wpState;
	
	EsButtonOnCommand(applyButton, [] (EsInstance *, EsElement *element, EsCommand *) {
		WallpaperState *wpState = (WallpaperState *) ((EsButton *) element)->userData.p;
		
		size_t pathBytes;
		char *pathText = EsTextboxGetContents(wpState->textbox, &pathBytes);
		
		if (pathText && pathBytes > 0) {
			EsMutexAcquire(&api.systemConfigurationMutex);
			char *valueCopy = (char *) EsHeapAllocate(pathBytes, false);
			EsMemoryCopy(valueCopy, pathText, pathBytes);
			SettingsPutValue("general", "wallpaper", valueCopy, pathBytes, nullptr, nullptr, false, true);
			
			if (wpState->fullPath) EsHeapFree(wpState->fullPath);
			wpState->fullPath = (char *) EsHeapAllocate(pathBytes, false);
			EsMemoryCopy(wpState->fullPath, pathText, pathBytes);
			wpState->fullPathBytes = pathBytes;
			
			desktop.configurationModified = true;
			EsThreadInformation threadInfo = {};
			EsThreadCreate(WallpaperLoad, &threadInfo, 0);
			EsMutexRelease(&api.systemConfigurationMutex);
			
			EsElementSetDisabled(wpState->applyButton, true);
		}
		
		EsHeapFree(pathText);
	});
	
	textbox->messageUser = [] (EsElement *element, EsMessage *message) {
		WallpaperState *wpState = (WallpaperState *) ((EsTextbox *) element)->userData.p;
		
		if (message->type == LT_MSG_TEXTBOX_UPDATED) {
			size_t currentBytes;
			char *currentText = EsTextboxGetContents(wpState->textbox, &currentBytes);
			
			bool different = false;

			if (wpState->fullPath) {
				if (wpState->fullPathBytes != currentBytes || EsMemoryCompare(wpState->fullPath, currentText, currentBytes) != 0) {
					different = true;
				}
			} else if (currentBytes > 0) {
				different = true;
			}
			
			EsElementSetDisabled(wpState->applyButton, !different);
			EsHeapFree(currentText);
			return LT_HANDLED;
		}
		
		return 0;
	};

	EsTextDisplayCreate(table, LT_CELL_H_RIGHT | LT_CELL_V_CENTER, 0, INTERFACE_STRING(DesktopSettingsThemeWindowColor));
	EsPanel *panel = EsPanelCreate(table, LT_CELL_H_LEFT | LT_PANEL_HORIZONTAL | LT_PANEL_RADIO_GROUP);
	uint8_t windowColor = EsSystemConfigurationReadInteger(EsLiteral("general"), EsLiteral("window_color"));

	for (uintptr_t i = 0; i < sizeof(windowColors) / sizeof(windowColors[0]); i++) {
		EsButton *button = EsButtonCreate(panel, LT_CELL_H_LEFT | LT_BUTTON_RADIOBOX | LT_ELEMENT_NO_FOCUS_ON_CLICK);
		if (windowColor == i) EsButtonSetCheck(button, LT_CHECK_CHECKED);
		button->userData.u = i;
		button->messageUser = SettingsColorButtonMessage;
		EsButtonOnCommand(button, SettingsColorButtonCommand);
	}

	table = EsPanelCreate(container, LT_CELL_H_FILL, EsStyleIntern(&styleSettingsCheckboxGroup));
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsThemeEnableHoverState), 'H', "general", "enable_hover_state");
	SettingsAddCheckbox(table, INTERFACE_STRING(DesktopSettingsThemeEnableAnimations), 'M', "general", "enable_animations");
}

SettingsPage settingsPages[] = {
	{ INTERFACE_STRING(DesktopSettingsDisplay), LT_ICON_PREFERENCES_DESKTOP_DISPLAY, SettingsPageDisplay, 'D' },
	{ INTERFACE_STRING(DesktopSettingsFiles), LT_ICON_USER_HOME_OPEN, SettingsPageFiles, 'S' },
	{ INTERFACE_STRING(DesktopSettingsKeyboard), LT_ICON_INPUT_KEYBOARD, SettingsPageKeyboard, 'E' },
	{ INTERFACE_STRING(DesktopSettingsMouse), LT_ICON_INPUT_MOUSE, SettingsPageMouse, 'G' },
	{ INTERFACE_STRING(DesktopSettingsTheme), LT_ICON_APPLICATIONS_INTERFACEDESIGN, SettingsPageTheme, 'M' },
	{ "Time & Date", 11, LT_ICON_PREFERENCES_SYSTEM_TIME, SettingsPageTime, 'T' },
};

void SettingsButtonPressed(EsInstance *_instance, EsElement *element, EsCommand *) {
	SettingsInstance *instance = (SettingsInstance *) _instance;
	SettingsPage *page = (SettingsPage *) element->userData.p;

	EsPanel *stack = EsPanelCreate(instance->switcher, LT_CELL_FILL | LT_PANEL_Z_STACK);
	page->create(stack, page);

	EsPanelSwitchTo(instance->switcher, stack, LT_TRANSITION_ZOOM_IN, LT_FLAGS_DEFAULT, 1.0f);
	EsElementSetDisabled(instance->backButton, false);
}

void InstanceSettingsCreate(EsMessage *message) {
	SettingsInstance *instance = (SettingsInstance *) _EsInstanceCreate(sizeof(SettingsInstance), message, nullptr);
	EsWindowSetTitle(instance->window, INTERFACE_STRING(DesktopSettingsTitle));
	EsWindowSetIcon(instance->window, LT_ICON_PREFERENCES_DESKTOP);
	EsPanel *windowBackground = EsPanelCreate(instance->window, LT_CELL_FILL, LT_STYLE_PANEL_WINDOW_BACKGROUND);
	instance->switcher = EsPanelCreate(windowBackground, LT_CELL_FILL | LT_PANEL_SWITCHER);
	EsPanel *content = EsPanelCreate(instance->switcher, LT_CELL_FILL | LT_PANEL_V_SCROLL_AUTO, EsStyleIntern(&styleNewTabContent));
	instance->mainPage = content;
	EsPanelSwitchTo(instance->switcher, content, LT_TRANSITION_NONE);

	{
		EsElement *toolbar = EsWindowGetToolbar(instance->window);

		EsButton *backButton = EsButtonCreate(toolbar, LT_BUTTON_TOOLBAR | LT_ELEMENT_STICKY_ACCESS_KEY, 0, INTERFACE_STRING(DesktopSettingsBackButton));
		instance->backButton = backButton;
		backButton->accessKey = 'A';
		EsButtonSetIcon(backButton, LT_ICON_GO_HOME_SYMBOLIC);
		EsButtonOnCommand(backButton, SettingsBackButton);
		EsElementSetDisabled(backButton, true);

		EsSpacerCreate(toolbar, LT_CELL_FILL);

		EsButton *undoButton = EsButtonCreate(toolbar, LT_BUTTON_TOOLBAR | LT_ELEMENT_STICKY_ACCESS_KEY, 0, INTERFACE_STRING(DesktopSettingsUndoButton));
		instance->undoButton = undoButton;
		undoButton->accessKey = 'U';
		EsButtonSetIcon(undoButton, LT_ICON_EDIT_UNDO_SYMBOLIC);
		EsButtonOnCommand(undoButton, SettingsUndoButton);
		EsElementSetDisabled(undoButton, true);
		EsElementSetHidden(undoButton, true);
	}

	{
		EsPanel *row = EsPanelCreate(content, LT_CELL_H_CENTER | LT_PANEL_HORIZONTAL);
		EsIconDisplayCreate(row, LT_FLAGS_DEFAULT, LT_STYLE_ICON_DISPLAY, LT_ICON_PREFERENCES_DESKTOP);
		EsSpacerCreate(row, LT_FLAGS_DEFAULT, 0, 10, 0);
		EsTextDisplayCreate(row, LT_FLAGS_DEFAULT, LT_STYLE_TEXT_HEADING1, INTERFACE_STRING(DesktopSettingsTitle));
	}

	{
		EsPanel *container = EsPanelCreate(content, LT_CELL_H_SHRINK | LT_PANEL_TABLE | LT_PANEL_HORIZONTAL, EsStyleIntern(&styleAllSettingsGroupContainer));
		EsPanelSetBands(container, 4);

		EsSort(settingsPages, sizeof(settingsPages) / sizeof(settingsPages[0]), sizeof(settingsPages[0]), [] (const void *_a, const void *_b, EsGeneric) {
			SettingsPage *a = (SettingsPage *) _a, *b = (SettingsPage *) _b;
			return EsStringCompare(a->string, a->stringBytes, b->string, b->stringBytes);
		}, nullptr);

		for (uintptr_t i = 0; i < sizeof(settingsPages) / sizeof(settingsPages[0]); i++) {
			SettingsPage *page = &settingsPages[i];
			EsButton *button = EsButtonCreate(container, LT_ELEMENT_NO_FOCUS_ON_CLICK | LT_CELL_H_FILL | LT_ELEMENT_STICKY_ACCESS_KEY, 
					EsStyleIntern(&styleSettingsButton), page->string, page->stringBytes);
			button->userData = page;
			button->accessKey = EsCRTisalpha(page->string[0]) ? page->string[0] : page->accessKey;
			EsButtonSetIcon(button, page->iconID);
			EsButtonOnCommand(button, SettingsButtonPressed);
		}
	}

	instance->callback = [] (EsInstance *, EsMessage *message) {
		if (message->type == LT_MSG_INSTANCE_DESTROY) {
			ConfigurationWriteToFile();
		}

		return 0;
	};
}
