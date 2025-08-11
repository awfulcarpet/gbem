#include <stdint.h>
enum {
	JOYP = 0xff00,
};

enum Buttons {
	SSBA = 1 << 5,
	DPAD = 1 << 4,

	BUTTON_START = 1 << 3,
	BUTTON_SELECT = 1 << 2,
	BUTTON_B = 1 << 1,
	BUTTON_A = 1 << 0,

	DPAD_DOWN = 1 << 3,
	DPAD_UP = 1 << 2,
	DPAD_LEFT = 1 << 1,
	DPAD_RIGHT = 1 << 0,
};

uint8_t get_input(uint8_t joypad);
