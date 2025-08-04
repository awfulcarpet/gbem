#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include "graphics.h"
#include "mem.h"

enum {
	BLACK = 0x00,
	DGRAY = 0x55,
	GRAY = 0xAA,
	WHITE = 0xff,
};

#define VRAM_TILE 0x8000

struct Tile {
	uint8_t pixels[8][8];
};

struct Tile *
get_tile(struct PPU *ppu, uint8_t id)
{
	struct Tile *t = calloc(1, sizeof(struct Tile));
	uint8_t h = 0, l = 0;
	uint16_t adr = VRAM_TILE + id * 16;
	for (int i = 0; i < 16; i++) {
		h = mem_read(ppu->mem, adr + i);
		fprintf(stderr, "%02x ", h);
	}
		fprintf(stderr, "\n");
	return NULL;
}

struct PPU *
graphics_init(uint8_t *mem)
{
	struct PPU *ppu = calloc(1, sizeof(struct PPU));

	if (ppu == NULL)
		return NULL;

	if (mem != NULL)
		ppu->mem = mem;

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
	// ppu->fb[SCREEN_WIDTH * SCREEN_HEIGHT / 2 + SCREEN_WIDTH / 2] = 0xFFFFFF;

	get_tile(ppu, 33);
	SDL_UpdateWindowSurface(ppu->win);
	return 0;
}
