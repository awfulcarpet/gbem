#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include "graphics.h"

struct PPU *
graphics_init(void)
{
	struct PPU *ppu = calloc(1, sizeof(struct PPU));

	if (ppu == NULL)
		return NULL;

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "unable to init SDL: %s\n", SDL_GetError());
		return NULL;
	}

	/* TODO: SCALE */
	ppu->win = SDL_CreateWindow("gbem", 0, 0, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);

	if (ppu->win == NULL) {
		fprintf(stderr, "unable to create sdl win: %s\n", SDL_GetError());
		return NULL;
	}


	ppu->fb = SDL_GetWindowSurface(ppu->win)->pixels;
	memset(ppu->fb, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
	SDL_UpdateWindowSurface(ppu->win);

	return ppu;
}

static void
set_ppu_mode(struct PPU *ppu, enum PPU_MODE mode)
{
	ppu->mode.mode = mode;

	switch (mode) {
		case OAM_SCAN:
			ppu->mode.dur = 80;
		break;
		case DRAW:
			ppu->mode.dur = 80;
		break;
		case HBLANK:
			ppu->mode.dur = 376;
		break;
		case VBLANK:
			ppu->mode.dur = 4560;
		break;
		default:
		assert(NULL); /* unreachable */
		break;
	}
}

int
graphics_scanline(struct PPU *ppu)
{
	set_ppu_mode(ppu, OAM_SCAN);
	ppu->fb[SCREEN_WIDTH * SCREEN_HEIGHT / 2 + SCREEN_WIDTH / 2] = 0xFFFFFF;


	SDL_UpdateWindowSurface(ppu->win);
	return 0;
}
