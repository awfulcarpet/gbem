#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include "gb.h"
#include "joypad.h"

void
get_input(struct GB *gb)
{
	SDL_Event e;
	SDL_PollEvent(&e);
	switch (e.type) {
		case SDL_KEYDOWN:
		{
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
						gb->running = 0;
						break;
				}
				break;
		}
		case SDL_KEYUP:
		{
				switch (e.key.keysym.sym) {
				}
				break;
		}
		case SDL_QUIT:
			gb->running = 0;
		break;
	}
}

