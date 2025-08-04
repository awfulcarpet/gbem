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
	uint8_t id;
};

struct Window {
	struct Tile *tiles[32][32];
};

struct Tile *
get_tile(struct PPU *ppu, uint8_t id)
{
	struct Tile *t = calloc(1, sizeof(struct Tile));
	if (t == NULL)
		return NULL;
	t->id = id;

	/* https://gbdev.io/pandocs/Tile_Data.html#data-format */
	uint8_t h = 0, l = 0;
	uint16_t adr = VRAM_TILE + id * 16;

	for (int i = 0; i < 16 - 1; i += 2) {
		l = mem_read(ppu->mem, adr + i);
		h = mem_read(ppu->mem, adr + i + 1);

		for (int j = 0; j < 8; j++) {
			uint8_t b1 = h & (1 << (7 - j)) ? 1 : 0;
			uint8_t b2 = l & (1 << (7 - j)) ? 1 : 0;
			uint8_t res = b1 << 1 | b2;
			t->pixels[i/2][j] = res;
		}
	}
	return t;
}

/* TODO: handle LCDC */
struct Window *
get_window(struct PPU *ppu)
{
	struct Window *w = calloc(1, sizeof(struct Window));
	if (w == NULL) return NULL;

	for (int i = 0; i < 32 * 32; i++) {
		w->tiles[i / 32][i % 32] = get_tile(ppu, mem_read(ppu->mem, i + 0x9880));
	}

	return w;
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

/* TODO: replace with scan line */
void
draw_tile(struct PPU *ppu, struct Tile *t, uint8_t x, uint8_t y)
{
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			uint8_t pix = t->pixels[i][j];
			uint32_t color = 0x00;
			switch (pix) {
				case 0:
					color = 0xFFFFFF;
				break;
				case 1:
					color = 0xAAAAAA;
				break;
				case 2:
					color = 0x555555;
				break;
				case 3:
					color = 0x00;
				break;
				default:
				assert(NULL); /* unreachable */
				break;
			}
			ppu->fb[i * SCREEN_WIDTH + j] = color;
		}
	}
}

int
graphics_scanline(struct PPU *ppu)
{
	set_ppu_mode(ppu, OAM_SCAN);
	// ppu->fb[SCREEN_WIDTH * SCREEN_HEIGHT / 2 + SCREEN_WIDTH / 2] = 0xFFFFFF;


	struct Tile *t = get_tile(ppu, 33);
	struct Window *w = get_window(ppu);

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			fprintf(stderr, "%d ", t->pixels[i][j]);
		}
		fprintf(stderr, "\n");
	}
		fprintf(stderr, "\n");
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 20; j++) {
			fprintf(stderr, "%3d ", w->tiles[i][j]->id);
		}
		fprintf(stderr, "\n");
	}
		draw_tile(ppu, w->tiles[0][15], 0 * 16, 0 * 16);
	SDL_UpdateWindowSurface(ppu->win);
	return 0;
}

