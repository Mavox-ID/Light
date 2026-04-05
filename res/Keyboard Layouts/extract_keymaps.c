#include <X11/Xlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// It looks like LT_SCANCODE_PUNCTUATION_1 and LT_SCANCODE_PUNCTUATION_2 are mutually exclusive 
// (_1 only on US keyboards, and _2 on everything else),
// and X11 merges them into key code 0x33.

#define LT_SCANCODE_A				(0x04)
#define LT_SCANCODE_B				(0x05)
#define LT_SCANCODE_C				(0x06)
#define LT_SCANCODE_D				(0x07)
#define LT_SCANCODE_E				(0x08)
#define LT_SCANCODE_F				(0x09)
#define LT_SCANCODE_G				(0x0A)
#define LT_SCANCODE_H				(0x0B)
#define LT_SCANCODE_I				(0x0C)
#define LT_SCANCODE_J				(0x0D)
#define LT_SCANCODE_K				(0x0E)
#define LT_SCANCODE_L				(0x0F)
#define LT_SCANCODE_M				(0x10)
#define LT_SCANCODE_N				(0x11)
#define LT_SCANCODE_O				(0x12)
#define LT_SCANCODE_P				(0x13)
#define LT_SCANCODE_Q				(0x14)
#define LT_SCANCODE_R				(0x15)
#define LT_SCANCODE_S				(0x16)
#define LT_SCANCODE_T				(0x17)
#define LT_SCANCODE_U				(0x18)
#define LT_SCANCODE_V				(0x19)
#define LT_SCANCODE_W				(0x1A)
#define LT_SCANCODE_X				(0x1B)
#define LT_SCANCODE_Y				(0x1C)
#define LT_SCANCODE_Z				(0x1D)
#define LT_SCANCODE_1				(0x1E)
#define LT_SCANCODE_2				(0x1F)
#define LT_SCANCODE_3				(0x20)
#define LT_SCANCODE_4				(0x21)
#define LT_SCANCODE_5				(0x22)
#define LT_SCANCODE_6				(0x23)
#define LT_SCANCODE_7				(0x24)
#define LT_SCANCODE_8				(0x25)
#define LT_SCANCODE_9				(0x26)
#define LT_SCANCODE_0				(0x27)
#define LT_SCANCODE_ENTER 			(0x28)
#define LT_SCANCODE_ESCAPE			(0x29)
#define LT_SCANCODE_BACKSPACE			(0x2A)
#define LT_SCANCODE_TAB				(0x2B)
#define LT_SCANCODE_SPACE			(0x2C)
#define LT_SCANCODE_HYPHEN			(0x2D)
#define LT_SCANCODE_EQUALS			(0x2E)
#define LT_SCANCODE_LEFT_BRACE			(0x2F)
#define LT_SCANCODE_RIGHT_BRACE			(0x30)
#define LT_SCANCODE_COMMA			(0x36)
#define LT_SCANCODE_PERIOD			(0x37)
#define LT_SCANCODE_SLASH			(0x38)
#define LT_SCANCODE_PUNCTUATION_1		(0x31) // On US keyboard, \|
#define LT_SCANCODE_PUNCTUATION_2		(0x32) // Not on US keyboard
#define LT_SCANCODE_PUNCTUATION_3		(0x33) // On US keyboard, ;:
#define LT_SCANCODE_PUNCTUATION_4		(0x34) // On US keyboard, '"
#define LT_SCANCODE_PUNCTUATION_5		(0x35) // On US keyboard, `~
#define LT_SCANCODE_PUNCTUATION_6		(0x64) // Not on US keyboard
#define LT_SCANCODE_F1				(0x3A)
#define LT_SCANCODE_F2				(0x3B)
#define LT_SCANCODE_F3				(0x3C)
#define LT_SCANCODE_F4				(0x3D)
#define LT_SCANCODE_F5				(0x3E)
#define LT_SCANCODE_F6				(0x3F)
#define LT_SCANCODE_F7				(0x40)
#define LT_SCANCODE_F8				(0x41)
#define LT_SCANCODE_F9				(0x42)
#define LT_SCANCODE_F10				(0x43)
#define LT_SCANCODE_F11				(0x44)
#define LT_SCANCODE_F12				(0x45)
#define LT_SCANCODE_CAPS_LOCK			(0x39)
#define LT_SCANCODE_PRINT_SCREEN		(0x46)
#define LT_SCANCODE_SCROLL_LOCK			(0x47)
#define LT_SCANCODE_PAUSE			(0x48)
#define LT_SCANCODE_INSERT			(0x49)
#define LT_SCANCODE_HOME			(0x4A)
#define LT_SCANCODE_PAGE_UP			(0x4B)
#define LT_SCANCODE_DELETE			(0x4C)
#define LT_SCANCODE_END				(0x4D)
#define LT_SCANCODE_PAGE_DOWN			(0x4E)
#define LT_SCANCODE_RIGHT_ARROW			(0x4F)
#define LT_SCANCODE_LEFT_ARROW			(0x50)
#define LT_SCANCODE_DOWN_ARROW			(0x51)
#define LT_SCANCODE_UP_ARROW			(0x52)
#define LT_SCANCODE_NUM_LOCK			(0x53)
#define LT_SCANCODE_HANGUL_ENGLISH_TOGGLE	(0x90)
#define LT_SCANCODE_HANJA_CONVERSION		(0x91)
#define LT_SCANCODE_KATAKANA			(0x92)
#define LT_SCANCODE_HIRAGANA			(0x93)
#define LT_SCANCODE_HANKAKU_ZENKAKU_TOGGLE	(0x94)
#define LT_SCANCODE_ALTERNATE_ERASE		(0x99)
#define LT_SCANCODE_NUM_DIVIDE			(0x54)
#define LT_SCANCODE_NUM_MULTIPLY		(0x55)
#define LT_SCANCODE_NUM_SUBTRACT		(0x56)
#define LT_SCANCODE_NUM_ADD			(0x57)
#define LT_SCANCODE_NUM_ENTER			(0x58)
#define LT_SCANCODE_NUM_1			(0x59)
#define LT_SCANCODE_NUM_2			(0x5A)
#define LT_SCANCODE_NUM_3			(0x5B)
#define LT_SCANCODE_NUM_4			(0x5C)
#define LT_SCANCODE_NUM_5			(0x5D)
#define LT_SCANCODE_NUM_6			(0x5E)
#define LT_SCANCODE_NUM_7			(0x5F)
#define LT_SCANCODE_NUM_8			(0x60)
#define LT_SCANCODE_NUM_9			(0x61)
#define LT_SCANCODE_NUM_0			(0x62)
#define LT_SCANCODE_NUM_POINT			(0x63)
#define LT_SCANCODE_NUM_EQUALS			(0x67)
#define LT_SCANCODE_NUM_TAB			(0xBA)
#define LT_SCANCODE_NUM_BACKSPACE		(0xBB)
#define LT_SCANCODE_MM_MUTE			(0x7F)
#define LT_SCANCODE_MM_LOUDER			(0x80)
#define LT_SCANCODE_MM_QUIETER			(0x81)
#define LT_SCANCODE_LEFT_CTRL			(0xE0)
#define LT_SCANCODE_LEFT_SHIFT			(0xE1)
#define LT_SCANCODE_LEFT_ALT			(0xE2)
#define LT_SCANCODE_RIGHT_CTRL			(0xE4)
#define LT_SCANCODE_RIGHT_SHIFT			(0xE5)
#define LT_SCANCODE_RIGHT_ALT			(0xE6)
#define LT_SCANCODE_ACPI_POWER 			(0x100)

uint32_t remap[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, LT_SCANCODE_ESCAPE, LT_SCANCODE_1, LT_SCANCODE_2, LT_SCANCODE_3, LT_SCANCODE_4, LT_SCANCODE_5, LT_SCANCODE_6,
	LT_SCANCODE_7, LT_SCANCODE_8, LT_SCANCODE_9, LT_SCANCODE_0, LT_SCANCODE_HYPHEN, LT_SCANCODE_EQUALS, LT_SCANCODE_BACKSPACE, LT_SCANCODE_TAB,
	LT_SCANCODE_Q, LT_SCANCODE_W, LT_SCANCODE_E, LT_SCANCODE_R, LT_SCANCODE_T, LT_SCANCODE_Y, LT_SCANCODE_U, LT_SCANCODE_I,
	LT_SCANCODE_O, LT_SCANCODE_P, LT_SCANCODE_LEFT_BRACE, LT_SCANCODE_RIGHT_BRACE, LT_SCANCODE_ENTER, LT_SCANCODE_LEFT_CTRL, LT_SCANCODE_A, LT_SCANCODE_S,
	LT_SCANCODE_D, LT_SCANCODE_F, LT_SCANCODE_G, LT_SCANCODE_H, LT_SCANCODE_J, LT_SCANCODE_K, LT_SCANCODE_L, LT_SCANCODE_PUNCTUATION_3, 
	LT_SCANCODE_PUNCTUATION_4, LT_SCANCODE_PUNCTUATION_5, LT_SCANCODE_LEFT_SHIFT, LT_SCANCODE_PUNCTUATION_1, LT_SCANCODE_Z, LT_SCANCODE_X, LT_SCANCODE_C, LT_SCANCODE_V,
	LT_SCANCODE_B, LT_SCANCODE_N, LT_SCANCODE_M, LT_SCANCODE_COMMA, LT_SCANCODE_PERIOD, LT_SCANCODE_SLASH, LT_SCANCODE_RIGHT_SHIFT, LT_SCANCODE_NUM_MULTIPLY,
	LT_SCANCODE_LEFT_ALT, LT_SCANCODE_SPACE, LT_SCANCODE_CAPS_LOCK, LT_SCANCODE_F1, LT_SCANCODE_F2, LT_SCANCODE_F3, LT_SCANCODE_F4, LT_SCANCODE_F5,
	LT_SCANCODE_F6, LT_SCANCODE_F7, LT_SCANCODE_F8, LT_SCANCODE_F9, LT_SCANCODE_F10, LT_SCANCODE_NUM_LOCK, LT_SCANCODE_SCROLL_LOCK, LT_SCANCODE_NUM_7,
	LT_SCANCODE_NUM_8, LT_SCANCODE_NUM_9, LT_SCANCODE_NUM_SUBTRACT, LT_SCANCODE_NUM_4, LT_SCANCODE_NUM_5, LT_SCANCODE_NUM_6, LT_SCANCODE_NUM_ADD, LT_SCANCODE_NUM_1, 
	LT_SCANCODE_NUM_2, LT_SCANCODE_NUM_3, LT_SCANCODE_NUM_0, LT_SCANCODE_NUM_POINT, 0, 0, LT_SCANCODE_PUNCTUATION_6, LT_SCANCODE_F11,
	LT_SCANCODE_F12, 0, LT_SCANCODE_KATAKANA, LT_SCANCODE_HIRAGANA, 0, 0, 0, 0,
	LT_SCANCODE_NUM_ENTER, LT_SCANCODE_RIGHT_CTRL, LT_SCANCODE_NUM_DIVIDE, LT_SCANCODE_PRINT_SCREEN, LT_SCANCODE_RIGHT_ALT, 0, LT_SCANCODE_HOME, LT_SCANCODE_UP_ARROW,
	LT_SCANCODE_PAGE_UP, LT_SCANCODE_LEFT_ARROW, LT_SCANCODE_RIGHT_ARROW, LT_SCANCODE_END, LT_SCANCODE_DOWN_ARROW, LT_SCANCODE_PAGE_DOWN, LT_SCANCODE_INSERT, LT_SCANCODE_DELETE,
	0, LT_SCANCODE_MM_MUTE, LT_SCANCODE_MM_QUIETER, LT_SCANCODE_MM_LOUDER, LT_SCANCODE_ACPI_POWER, LT_SCANCODE_NUM_EQUALS, 0, LT_SCANCODE_PAUSE,
};

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <layout>\n", argv[0]);
		return 1;
	}

	FILE *f = popen("setxkbmap -query | grep layout", "r");
	char oldLayout[64] = {};
	fread(oldLayout, 1, sizeof(oldLayout) - 1, f);
	pclose(f);

	char setLayout[128];
	snprintf(setLayout, sizeof(setLayout), "setxkbmap %s", argv[1]);
	system(setLayout);

	Display *display = XOpenDisplay(NULL);
	XIM xim = XOpenIM(display, 0, 0, 0);
	XSetLocaleModifiers("");
	XSetWindowAttributes attributes = {};
	Window window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 800, 600, 0, 0, 
			InputOutput, CopyFromParent, CWOverrideRedirect, &attributes);
	XIC xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing, XNClientWindow, window, XNFocusWindow, window, NULL);

	uint16_t table[512 * 4] = {};
	char stringBuffer[65536] = {};
	size_t stringBufferPosition = 0;

	for (uintptr_t state = 0; state < 4; state++) {
		for (uintptr_t i = 0; i < 0x80; i++) {
			XEvent event = {};
			event.xkey.type = KeyPress;
			event.xkey.display = display;
			event.xkey.window = window;
			event.xkey.state = state == 0 ? 0x00 : state == 1 ? 0x01 /* shift */ : state == 2 ? 0x80 /* alt gr */ : state == 3 ? 0x81 : 0;
			event.xkey.keycode = i;
			char text[32];
			KeySym symbol = NoSymbol;
			Status status;
			size_t textBytes = Xutf8LookupString(xic, &event.xkey, text, sizeof(text) - 1, &symbol, &status); 
			uint16_t offset = 0;

			if (textBytes) {
				offset = stringBufferPosition;
				assert(stringBufferPosition + textBytes + 1 < sizeof(stringBuffer));
				memcpy(stringBuffer + stringBufferPosition, text, textBytes);
				stringBuffer[stringBufferPosition + textBytes] = 0;
				stringBufferPosition += textBytes + 1;
			}

			table[remap[i] + state * 512] = offset;

			if (remap[i] == LT_SCANCODE_PUNCTUATION_1) {
				table[LT_SCANCODE_PUNCTUATION_2 + state * 512] = offset;
			}
		}
	}

	snprintf(setLayout, sizeof(setLayout), "setxkbmap %s", oldLayout + 7);
	system(setLayout);

	fwrite(table, 1, sizeof(table), stdout);
	fwrite(stringBuffer, 1, stringBufferPosition, stdout);

	return 0;
}
