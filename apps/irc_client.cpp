// TODO Don't use EsTextbox for the output..
// TODO Put the connection settings in a Panel.Popup.

#define LT_INSTANCE_TYPE Instance
#include <light.h>

struct Instance : EsInstance {
	EsTextbox *textboxNick;
	EsTextbox *textboxAddress;
	EsTextbox *textboxPort;
	EsTextbox *textboxOutput;
	EsTextbox *textboxInput;
	EsButton *buttonConnect;
	EsButton *buttonTest;

	EsThreadInformation networkingThread;
	EsThreadInformation testThread;

	EsMutex inputCommandMutex;
	char *inputCommand;
	size_t inputCommandBytes;
};

const EsStyle styleSmallTextbox = {
	.inherit = LT_STYLE_TEXTBOX_BORDERED_SINGLE,

	.metrics = {
		.mask = LT_THEME_METRICS_PREFERRED_WIDTH,
		.preferredWidth = 100,
	},
};

const EsStyle styleOutputTextbox = {
	.inherit = LT_STYLE_TEXTBOX_NO_BORDER,

	.metrics = {
		.mask = LT_THEME_METRICS_FONT_FAMILY,
		.fontFamily = LT_FONT_MONOSPACED,
	},
};

const EsStyle styleInputTextbox = {
	.inherit = LT_STYLE_TEXTBOX_BORDERED_SINGLE,

	.metrics = {
		.mask = LT_THEME_METRICS_FONT_FAMILY,
		.fontFamily = LT_FONT_MONOSPACED,
	},
};

int TextboxInputCallback(EsElement *element, EsMessage *message) {
	Instance *instance = element->instance;

	if (message->type == LT_MSG_KEY_DOWN) {
		if (message->keyboard.scancode == LT_SCANCODE_ENTER) {
			size_t inputCommandBytes = 0;
			char *inputCommand = EsTextboxGetContents(instance->textboxInput, &inputCommandBytes);

			if (inputCommandBytes) {
				EsMutexAcquire(&instance->inputCommandMutex);
				EsHeapFree(instance->inputCommand);
				instance->inputCommand = inputCommand;
				instance->inputCommandBytes = inputCommandBytes;
				EsMutexRelease(&instance->inputCommandMutex);
			} else {
				EsHeapFree(inputCommand);
			}

			EsTextboxClear(instance->textboxInput, false);
			return LT_HANDLED;
		}
	}

	return 0;
}

void NetworkingThread(EsGeneric argument) {
	Instance *instance = (Instance *) argument.p;

	char errorMessage[512];
	size_t errorMessageBytes = 0;

	char message[4096];
	size_t messageBytes = 0;

	char nick[64];
	size_t nickBytes = 0;

	char *password = nullptr;
	size_t passwordBytes = 0;

	char *address = nullptr;
	size_t addressBytes = 0;

	char buffer[1024];
	uintptr_t bufferPosition = 0;
	
	EsConnection connection = {};
	connection.sendBufferBytes = 8192;
	connection.receiveBufferBytes = 65536;

	{
		EsMessageMutexAcquire();
		address = EsTextboxGetContents(instance->textboxAddress, &addressBytes);
		EsMessageMutexRelease();
		EsError error = EsAddressResolve(address, addressBytes, LT_FLAGS_DEFAULT, &connection.address);

		if (error != LT_SUCCESS) {
                      if (address) {
                          errorMessageBytes = EsStringFormat(errorMessage, sizeof(errorMessage), 
                              "The address name '%z' could not be found.\n", addressBytes, address);
                      } else {
                          errorMessageBytes = EsStringFormat(errorMessage, sizeof(errorMessage), "Invalid address.\n");
                      }
                      goto exit;
                  }
	}

	{
		EsMessageMutexAcquire();
		size_t portBytes;
		char *port = EsTextboxGetContents(instance->textboxPort, &portBytes);
		EsMessageMutexRelease();
		connection.address.port = EsIntegerParse(port, portBytes); 
		EsHeapFree(port);
	}

	{
		EsMessageMutexAcquire();
		char *_nick = EsTextboxGetContents(instance->textboxNick, &nickBytes);
		EsMessageMutexRelease();
		if (nickBytes > sizeof(nick)) nickBytes = sizeof(nick);
		if (_nick && nickBytes > 0) {
			EsMemoryCopy(nick, _nick, nickBytes);
		} else {
			nickBytes = 0;
			nick[0] = 0;
		}
		EsHeapFree(_nick);

		for (uintptr_t i = 0; i < nickBytes; i++) {
			if (nick[i] == ':') {
				password = nick + i + 1;
				passwordBytes = nickBytes - i - 1;
				nickBytes = i;
			}
		}
	}

	{
		EsError error = EsConnectionOpen(&connection, LT_CONNECTION_OPEN_WAIT);

		if (error != LT_SUCCESS) {
			errorMessageBytes = EsStringFormat(errorMessage, sizeof(errorMessage), 
					"Could not open the connection (%d).", error);
			goto exit;
		}

                if (password) {
		messageBytes = EsStringFormat(message, sizeof(message), "PASS %z\r\n", passwordBytes, password);
		EsConnectionWriteSync(&connection, message, messageBytes);
	}

	        messageBytes = EsStringFormat(message, sizeof(message), 
			        "NICK %z\r\nUSER %z localhost %z :%z\r\n",
			        nickBytes, nick, nickBytes, nick, addressBytes, address, nickBytes, nick);
	        EsConnectionWriteSync(&connection, message, messageBytes);
	}

	while (true) {
		// TODO Ping the server every 2 minutes.
		// TODO If we've received no messages for 5 minutes, timeout.

		uintptr_t inputBytes = 0;

		while (true) {
			char *inputCommand = nullptr;
			size_t inputCommandBytes = 0;

			EsMutexAcquire(&instance->inputCommandMutex);
			inputCommand = instance->inputCommand;
			inputCommandBytes = instance->inputCommandBytes;
			instance->inputCommand = nullptr;
			instance->inputCommandBytes = 0;
			EsMutexRelease(&instance->inputCommandMutex);

			if (inputCommand) {
				messageBytes = EsStringFormat(message, sizeof(message), "%z\r\n", inputCommandBytes, inputCommand);
				EsConnectionWriteSync(&connection, message, messageBytes);
				EsHeapFree(inputCommand);
			}

			size_t bytesRead = 0;
			EsError error = EsConnectionRead(&connection, buffer + bufferPosition, sizeof(buffer) - bufferPosition, &bytesRead);
			if (error != LT_SUCCESS || bytesRead == 0) {
				goto exit;
			}

			bufferPosition += bytesRead;

			if (bufferPosition >= 2) {
				for (uintptr_t i = 0; i < bufferPosition - 1; i++) {
					if (buffer[i] == '\r' && buffer[i + 1] == '\n') {
						buffer[i] = 0;
						inputBytes = i + 2;
						goto gotMessage;
					}
				}
			}

			if (bufferPosition == sizeof(buffer)) {
				errorMessageBytes = EsStringFormat(errorMessage, sizeof(errorMessage), "The server sent an invalid message.");
				goto exit;
			}
		}

		gotMessage:;

		{
			const char *command = nullptr, *user = nullptr, *parameters = nullptr, *text = nullptr;
			char *position = buffer;

			if (*position == ':') {
				user = ++position;
				while (*position && *position != ' ' && *position != '!') position++;
				if (*position) *position++ = 0;
			} else {
				user = address;
			}

			while (*position && *position == ' ') position++;
			command = position;
			while (*position && *position != ' ') position++;
			if (*position) *position++ = 0;
			while (*position && *position == ' ') position++;

			if (*position != ':') {
				parameters = position;
				while (*position && *position != ' ') position++;
				if (*position) *position++ = 0;
			}

			while (*position && *position == ' ') position++;
			if (*position == ':') text = position + 1;

			if (0 == EsCRTstrcmp(command, "PING")) {
				messageBytes = EsStringFormat(message, sizeof(message), "PONG :%z\r\n", text ?: parameters);
				EsConnectionWriteSync(&connection, message, messageBytes);
			} else {
				char displayBuffer[1024];
				
				if (0 == EsCRTstrcmp(command, "PRIVMSG")) {
					EsStringFormat(displayBuffer, sizeof(displayBuffer), "[%z] %z: %z\n", parameters, user, text);
				} else {
					EsStringFormat(displayBuffer, sizeof(displayBuffer), "* %z %z\n", command, text ?: parameters);
				}

				EsMessageMutexAcquire();
				EsTextboxInsert(instance->textboxOutput, displayBuffer);
				EsMessageMutexRelease();
			}
		}

                                EsMemoryMove(buffer, buffer + inputBytes, bufferPosition - inputBytes, false);
                                bufferPosition -= inputBytes;
	}

	exit:;

	if (connection.handle) {
		EsConnectionClose(&connection);
	}

	EsMessageMutexAcquire();

	if (errorMessageBytes) {
		EsDialogShow(instance->window, EsLiteral("Connection failed"), errorMessage, errorMessageBytes, 
				LT_ICON_DIALOG_ERROR, LT_DIALOG_ALERT_OK_BUTTON); 
	}

	EsElementSetDisabled(instance->textboxAddress, false);
	EsElementSetDisabled(instance->textboxNick, false);
	EsElementSetDisabled(instance->textboxPort, false);
	EsElementSetDisabled(instance->textboxInput, true);
	EsElementSetDisabled(instance->buttonConnect, false);

	EsMessageMutexRelease();

	EsHeapFree(address);
}

void TestThread(EsGeneric argument) {
	Instance *inst = (Instance *)argument.p;
	char msg[128];

	EsMessageMutexAcquire();
	EsTextboxInsert(inst->textboxOutput, "--- Internet Test ---\nConnecting...\n");
	EsMessageMutexRelease();

	EsConnection conn = {};
	if (EsAddressResolve("example.com", 11, LT_FLAGS_DEFAULT, &conn.address) == LT_SUCCESS) {
		conn.address.port = 80;
		if (EsConnectionOpen(&conn, LT_CONNECTION_OPEN_WAIT) == LT_SUCCESS) {
			const char *req = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n";
			EsConnectionWriteSync(&conn, req, 56);

			char buf[512];
			size_t readBytes = 0, total = 0;
			while (EsConnectionRead(&conn, buf, sizeof(buf), &readBytes) == LT_SUCCESS && readBytes > 0) {
				total += readBytes;
			}

			EsStringFormat(msg, sizeof(msg), "Test end. Loaded bytes: %d\n", (int)total);
			
			EsMessageMutexAcquire();
			EsTextboxInsert(inst->textboxOutput, msg);
			EsMessageMutexRelease();
			EsConnectionClose(&conn);
		} else {
			EsMessageMutexAcquire();
			EsTextboxInsert(inst->textboxOutput, "Error: Open failed.\n");
			EsMessageMutexRelease();
		}
	}
}

void ConnectCommand(Instance *instance, EsElement *, EsCommand *) {
	EsElementSetDisabled(instance->textboxAddress, true);
	EsElementSetDisabled(instance->textboxNick, true);
	EsElementSetDisabled(instance->textboxPort, true);
	EsElementSetDisabled(instance->textboxInput, false);
	EsElementSetDisabled(instance->buttonConnect, true);
	EsElementSetDisabled(instance->buttonTest, true);

	EsGeneric arg = { 0 };
	arg.p = instance;
	EsThreadCreate(NetworkingThread, &instance->networkingThread, arg);
}

void TestCommand(Instance *instance, EsElement *, EsCommand *) {
	EsGeneric arg = { 0 };
	arg.p = instance;
	EsThreadCreate(TestThread, &instance->testThread, arg);
}

void _start() {
	_init();

	while (true) {
		EsMessage *message = EsMessageReceive();

		if (message->type == LT_MSG_INSTANCE_CREATE) {
			// Create an new instance.

			Instance *instance = EsInstanceCreate(message, "IRC Client");
			EsWindow *window = instance->window;
			EsWindowSetIcon(window, LT_ICON_INTERNET_CHAT);

			// Create the toolbar.

			EsElement *toolbar = EsWindowGetToolbar(window);
			EsPanel *section = EsPanelCreate(toolbar, LT_PANEL_HORIZONTAL);
			EsTextDisplayCreate(section, LT_FLAGS_DEFAULT, 0, EsLiteral("Nick:"));
			instance->textboxNick = EsTextboxCreate(section, LT_FLAGS_DEFAULT, EsStyleIntern(&styleSmallTextbox));
			EsSpacerCreate(toolbar, LT_FLAGS_DEFAULT, 0, 5, 0);
			section = EsPanelCreate(toolbar, LT_PANEL_HORIZONTAL);
			EsTextDisplayCreate(section, LT_FLAGS_DEFAULT, 0, EsLiteral("Address:"));
			instance->textboxAddress = EsTextboxCreate(section, LT_FLAGS_DEFAULT, EsStyleIntern(&styleSmallTextbox));
			EsSpacerCreate(toolbar, LT_FLAGS_DEFAULT, 0, 5, 0);
			section = EsPanelCreate(toolbar, LT_PANEL_HORIZONTAL);
			EsTextDisplayCreate(section, LT_FLAGS_DEFAULT, 0, EsLiteral("Port:"));
			instance->textboxPort = EsTextboxCreate(section, LT_FLAGS_DEFAULT, EsStyleIntern(&styleSmallTextbox));
			EsSpacerCreate(toolbar, LT_CELL_H_FILL);
			instance->buttonTest = EsButtonCreate(toolbar, LT_FLAGS_DEFAULT, 0, EsLiteral("Net Test"));
			EsButtonOnCommand(instance->buttonTest, TestCommand);
			instance->buttonConnect = EsButtonCreate(toolbar, LT_FLAGS_DEFAULT, 0, EsLiteral("Connect"));
			EsButtonOnCommand(instance->buttonConnect, ConnectCommand);

			// Create the main area.

			EsPanel *panel = EsPanelCreate(window, LT_PANEL_VERTICAL | LT_CELL_FILL, LT_STYLE_PANEL_WINDOW_DIVIDER);
			instance->textboxOutput = EsTextboxCreate(panel, LT_CELL_FILL | LT_TEXTBOX_MULTILINE, EsStyleIntern(&styleOutputTextbox));
			EsPanelCreate(panel, LT_CELL_H_FILL, LT_STYLE_SEPARATOR_HORIZONTAL);
			EsPanel *inputArea = EsPanelCreate(panel, LT_PANEL_HORIZONTAL | LT_CELL_H_FILL, LT_STYLE_PANEL_FILLED);
			instance->textboxInput = EsTextboxCreate(inputArea, LT_CELL_FILL, EsStyleIntern(&styleInputTextbox));
			instance->textboxInput->messageUser = TextboxInputCallback;
			EsElementSetDisabled(instance->textboxInput);
		}
	}
}
