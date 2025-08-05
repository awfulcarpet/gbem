#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include "graphics.h"
#include "mem.h"

enum Color {
	BLACK = 0x000000,
	DGRAY = 0x555555,
	GRAY = 0xaaaaaa,
	WHITE = 0xffffff,
};

enum Pallete {
	BGP = 0xff47,
	OBP0 = 0xff48,
	OBP1 = 0xff49,
};

#define VRAM_TILE 0x8000

enum {
	BYTES_PER_SPRITE = 4,
	BYTES_PER_TILE = 16,
};

struct Tile {
	uint8_t pixels[8][8];
	uint8_t id;
};

struct Window {
	struct Tile *tiles[32][32];
};



struct Sprite {
	uint8_t tile_id;
	uint8_t x, y;
	struct {
		uint8_t priority:1;
		uint8_t yflip:1;
		uint8_t xflip:1;
		uint8_t dmg_palette:1;
		uint8_t pad:4;
	};
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
	uint16_t adr = VRAM_TILE + id * BYTES_PER_TILE;

	for (int i = 0; i < BYTES_PER_TILE - 1; i += 2) {
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

struct Sprite *
get_sprite(struct PPU *ppu, uint8_t id)
{
	uint16_t adr = OAM + id * BYTES_PER_SPRITE;
	struct Sprite *s = calloc(1, sizeof(struct Sprite));
	if (s == NULL)
		return NULL;

	s->y = mem_read(ppu->mem, adr) - 16;
	s->x = mem_read(ppu->mem, adr + 1) - 8;
	s->tile_id = mem_read(ppu->mem, adr + 2);

	uint8_t flag = mem_read(ppu->mem, adr + 3);

	s->priority = (flag & (1 << 7)) >> 7;
	s->yflip = (flag & (1 << 6)) >> 6;
	s->xflip = (flag & (1 << 5)) >> 5;
	s->dmg_palette = (flag & (1 << 4)) >> 4;

	return s;
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

	ppu->log = fopen("lcd", "w");


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

uint8_t
get_color(struct PPU *ppu, uint8_t id, enum Pallete pallete)
{
	return (mem_read(ppu->mem, pallete) & (3 << (id * 2))) >> (id * 2);
}

/* TODO: replace with scan line */
void
draw_tile(struct PPU *ppu, struct Tile *t, uint8_t xpix, uint8_t ypix, enum Pallete pallete)
{
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			uint8_t pix = t->pixels[i][j];
			uint32_t color = 0x00;
			switch (get_color(ppu, pix, pallete)) {
				case 0:
					if (pallete != BGP)
						continue;
					color = WHITE;
				break;
				case 1:
					color = GRAY;
				break;
				case 2:
					color = DGRAY;
				break;
				case 3:
					color = BLACK;
				break;
				default:
				assert(NULL); /* unreachable */
				break;
			}
			ppu->fb[ypix * SCREEN_WIDTH + i * SCREEN_WIDTH + j + xpix] = color;
		}
	}
}

void
tile_xflip(struct Tile *t)
{
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 4; j++) {
			t->pixels[i][j] ^= t->pixels[i][7-j];
			t->pixels[i][7-j] ^= t->pixels[i][j];
			t->pixels[i][j] ^= t->pixels[i][7-j];
		}
	}
}

void
tile_yflip(struct Tile *t)
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 8; j++) {
			t->pixels[i][j] ^= t->pixels[7-i][j];
			t->pixels[7-i][j] ^= t->pixels[i][j];
			t->pixels[i][j] ^= t->pixels[7-i][j];
		}
	}
}

void
sprite_render(struct PPU *ppu, struct Sprite *s)
{
	struct Tile *t = get_tile(ppu, s->tile_id);
	if (s->xflip)
		tile_xflip(t);
	if (s->yflip)
		tile_yflip(t);
	draw_tile(ppu, t, s->x, s->y, s->dmg_palette ? OBP1 : OBP0);
	free(t);
}


int
graphics_scanline(struct PPU *ppu)
{
	set_ppu_mode(ppu, OAM_SCAN);
	mem_write(ppu->mem, LY, mem_read(ppu->mem, LY) + 1);

	struct Window *w = get_window(ppu);
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 20; j++) {
			draw_tile(ppu, w->tiles[i][j], j * 8, i * 8, BGP);
		}
	}


	for (int i = 0; i < 40; i++) {
	struct Sprite *s = get_sprite(ppu, i);
	sprite_render(ppu, s);
		free(s);
	}
	SDL_UpdateWindowSurface(ppu->win);
	return 0;
}

void
graphics_log(struct PPU *ppu)
{
	fprintf(ppu->log, "LCDC: %08b ", mem_read(ppu->mem, LCDC));
	fprintf(ppu->log, "LY: %02x ", mem_read(ppu->mem, LY));
	fprintf(ppu->log, "LYC: %02x ", mem_read(ppu->mem, LYC));
	fprintf(ppu->log, "STAT: %07b ", mem_read(ppu->mem, STAT));
	fprintf(ppu->log, "\n");
}

