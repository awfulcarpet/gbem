#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include "mem.h"
#include "gb.h"
#include "joypad.h"

void
get_input(struct GB *gb)
{
	SDL_Event e;
	SDL_PollEvent(&e);
	uint8_t buttons = 0;
	uint8_t dpad = 0;
	uint8_t joypad = mem_read(gb->mem, JOYP);
	mem_write(gb->mem, JOYP, 0xcf);

	switch (e.type) {
		case SDL_KEYDOWN:
		{
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
						gb->running = 0;
						break;
					case SDLK_UP:
						buttons |= DPAD_UP;
					break;
					case SDLK_DOWN:
						buttons |= DPAD_DOWN;
					break;
					case SDLK_LEFT:
						buttons |= DPAD_LEFT;
					break;
					case SDLK_RIGHT:
						buttons |= DPAD_RIGHT;
					break;
				}
				break;
		}
		case SDL_KEYUP:
		{
				switch (e.key.keysym.sym) {
					case SDLK_UP:
						buttons &= ~DPAD_UP;
					break;
					case SDLK_DOWN:
						buttons &= ~DPAD_DOWN;
					break;
					case SDLK_LEFT:
						buttons &= ~DPAD_LEFT;
					break;
					case SDLK_RIGHT:
						buttons &= ~DPAD_RIGHT;
					break;
				}
				break;
		}
		case SDL_QUIT:
			gb->running = 0;
		break;
	}
}

