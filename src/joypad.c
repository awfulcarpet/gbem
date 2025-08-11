#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <stdint.h>
#include "mem.h"
#include "gb.h"
#include "joypad.h"
static uint8_t buttons = ~0;
static uint8_t dpad = ~0;

uint8_t
get_input(uint8_t joypad)
{
	SDL_Event e;
	SDL_PollEvent(&e);

	switch (e.type) {
		case SDL_KEYDOWN:
		{
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
						exit(0);
						break;
					case SDLK_RETURN:
						buttons &= ~BUTTON_START;
						break;

					case SDLK_UP:
						dpad &= ~DPAD_UP;
					break;
					case SDLK_DOWN:
						dpad &= ~DPAD_DOWN;
					break;
					case SDLK_LEFT:
						dpad &= ~DPAD_LEFT;
					break;
					case SDLK_RIGHT:
						dpad &= ~DPAD_RIGHT;
					break;
				}
				break;
		}
		case SDL_KEYUP:
		{
				switch (e.key.keysym.sym) {
					case SDLK_RETURN:
						buttons |= BUTTON_START;
						break;

					case SDLK_UP:
						dpad |= DPAD_UP;
					break;
					case SDLK_DOWN:
						dpad |= DPAD_DOWN;
					break;
					case SDLK_LEFT:
						dpad |= DPAD_LEFT;
					break;
					case SDLK_RIGHT:
						dpad |= DPAD_RIGHT;
					break;
				}
				break;
		}
		case SDL_QUIT:
			exit(0);
		break;
	}

	if ((joypad & SSBA) == 0) {
		return (joypad & 0xf0) | (buttons & 0xf);
	}

	if ((joypad & DPAD) == 0) {
		return (joypad & 0xf0) | (dpad & 0xf);
	}


	return (joypad & 0xf0) | 0xf;
}

