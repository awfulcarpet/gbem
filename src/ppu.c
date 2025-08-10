#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include <stdint.h>
#include "cpu.h"
#include "ppu.h"
#include "mem.h"

enum {
	LYC_INT = 1 << 6,
	MODE_2_INT = 1 << 5,
	MODE_1_INT = 1 << 4,
	MODE_0_INT = 1 << 3,
	LYC_LC = 1 << 2,
	MODE = 3,
};

static uint8_t wly = 0;
static uint8_t statline = 0;

void
request_stat(struct PPU *ppu)
{
	uint8_t ly = mem_read(ppu->mem, LY);
	uint8_t lyc = mem_read(ppu->mem, LYC);
	uint8_t stat = mem_read(ppu->mem, STAT);

	uint8_t line = 0;

	if (stat & LYC_INT) {
		line |= ((stat & LYC_LC) ? 1 : 0);
	}

	if (line != 0 && statline == 0) {
		request_interrupt(ppu->mem, INTERRUPT_STAT);
		fprintf(ppu->log, "int %d\n", INTERRUPT_STAT);
	}

	statline = line;
}

struct LCD_Control
read_lcdc(struct PPU *ppu)
{
	struct LCD_Control lcdc = {0};
	uint8_t tmp = mem_read(ppu->mem, LCDC);

	lcdc.enable = tmp & (1 << 7) ? 1 : 0;
	lcdc.w_tmap = tmp & (1 << 6) ? 1 : 0;
	lcdc.wenable = tmp & (1 << 5) ? 1 : 0;
	lcdc.tdata = tmp & (1 << 4) ? 1 : 0;
	lcdc.bg_tmap = tmp & (1 << 3) ? 1 : 0;
	lcdc.obj_size = tmp & (1 << 2) ? 1 : 0;
	lcdc.obj_enable = tmp & (1 << 1) ? 1 : 0;
	lcdc.bgwin_enable = tmp & (1 << 0) ? 1 : 0;

	return lcdc;
}

/* row is row in tile */
static uint8_t*
get_tile_row(struct PPU *ppu, uint8_t id, uint8_t row, enum TILE_TYPE type)
{
	assert(row < 8);
	uint8_t *pix = calloc(8, sizeof(uint8_t));

	uint8_t h = 0, l = 0;
	uint16_t adr = VRAM + id * BYTES_PER_TILE;

	if (type == WINDOW && ppu->lcdc.tdata == 0) {
		adr = 0x9000 + (int8_t)id * BYTES_PER_TILE;
	}

	l = mem_read(ppu->mem, adr + row * 2);
	h = mem_read(ppu->mem, adr + row * 2 + 1);

	for (int j = 0; j < 8; j++) {
		uint8_t b1 = h & (1 << (7 - j)) ? 1 : 0;
		uint8_t b2 = l & (1 << (7 - j)) ? 1 : 0;
		uint8_t res = b1 << 1 | b2;
		pix[j] = res;
	}

	return pix;
}

/* returns 20 tile ids for the window/bg */
uint8_t *
get_bg_row(struct PPU *ppu, uint16_t adr, uint8_t ly)
{
	uint8_t *row = calloc(20 + 1, sizeof(uint8_t *));
	uint8_t scy = mem_read(ppu->mem, SCY);
	uint8_t scx = mem_read(ppu->mem, SCX);

	for (int i = 0; i < LCD_WIDTH_TILES + 1; i++) {
		row[i] = mem_read(ppu->mem, adr + (ly + scy)/8 * WINDOW_WIDTH_TILES + (i + scx/8) % WINDOW_WIDTH_TILES);
	}

	return row;
}

/* returns 20 tile ids for the window/bg */
uint8_t *
get_window_row(struct PPU *ppu, uint16_t adr, uint8_t ly)
{
	uint8_t *row = calloc(WINDOW_WIDTH_TILES, sizeof(uint8_t *));
	uint8_t wy = mem_read(ppu->mem, WY);
	uint8_t wx = mem_read(ppu->mem, WX);

	for (int i = 0; i < WINDOW_WIDTH_TILES; i++) {
		row[i] = mem_read(ppu->mem, adr + (wly)/8 * WINDOW_WIDTH_TILES + i);
	}

	return row;
}


struct Sprite *
get_sprite(struct PPU *ppu, uint8_t id)
{
	uint16_t adr = OAM + id * BYTES_PER_SPRITE;
	struct Sprite *s = calloc(1, sizeof(struct Sprite));
	if (s == NULL)
		return NULL;

	s->y = mem_read(ppu->mem, adr);
	s->x = mem_read(ppu->mem, adr + 1);
	s->tile_id = mem_read(ppu->mem, adr + 2);

	uint8_t flag = mem_read(ppu->mem, adr + 3);

	s->priority = (flag & (1 << 7)) >> 7;
	s->yflip = (flag & (1 << 6)) >> 6;
	s->xflip = (flag & (1 << 5)) >> 5;
	s->dmg_palette = (flag & (1 << 4)) >> 4;

	return s;
}

static int
graphics_init(struct PPU *ppu)
{
	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}

	/* TODO: SCALE */
	ppu->win = SDL_CreateWindow("gbem", 0, 0, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);
	ppu->debug_win = SDL_CreateWindow("gbem background tiles", 0, 0, 8 * WINDOW_WIDTH_TILES, 8 * WINDOW_HEIGHT_TILES, SDL_WINDOW_SHOWN | SDL_WINDOW_UTILITY);

	if (ppu->win == NULL) {
		fprintf(stderr, "unable to create sdl win: %s\n", SDL_GetError());
		return 1;
	}
	if (ppu->debug_win == NULL) {
		fprintf(stderr, "unable to create sdl debug win: %s\n", SDL_GetError());
		return 1;
	}

	ppu->fb = SDL_GetWindowSurface(ppu->win)->pixels;
	ppu->debug_fb = SDL_GetWindowSurface(ppu->debug_win)->pixels;
	memset(ppu->fb, 0xff, SCREEN_WIDTH * SCREEN_HEIGHT * SCALE * SCALE * sizeof(uint32_t));

	SDL_UpdateWindowSurface(ppu->win);
	SDL_UpdateWindowSurface(ppu->debug_win);

	return 0;
}

struct PPU *
ppu_init(uint8_t *mem)
{
	struct PPU *ppu = calloc(1, sizeof(struct PPU));

	if (ppu == NULL)
		return NULL;

	if (mem != NULL)
		ppu->mem = mem;

	ppu->log = fopen("ppu.log", "w");

	if (ppu->log == NULL) {
		fprintf(stderr, "unable to open ppu log for writing\n");
		free(ppu);
		return NULL;
	}

	ppu->mode.mode = OAM_SCAN;

	/* https://bgb.bircd.org/pandocs.htm#powerupsequence */
	mem_write(ppu->mem, LCDC, 0x91);
	mem_write(ppu->mem, SCY, 0x00);
	mem_write(ppu->mem, SCX, 0x00);
	mem_write(ppu->mem, LYC, 0x00);
	mem_write(ppu->mem, BGP, 0xfc);
	mem_write(ppu->mem, OBP0, 0xff);
	mem_write(ppu->mem, OBP1, 0xff);
	mem_write(ppu->mem, WY, 0x00);
	mem_write(ppu->mem, WX, 0x00);


	if (graphics_init(ppu)) {
		free(ppu);
		return NULL;
	}

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
	mem_write(ppu->mem, STAT, mem_read(ppu->mem, STAT) | mode);
}

uint8_t
get_color(struct PPU *ppu, uint8_t id, enum Pallete pallete)
{
	return (mem_read(ppu->mem, pallete) & (3 << (id * 2))) >> (id * 2);
}

void
draw_tile_row(struct PPU *ppu, uint8_t *row, int16_t xpix, enum Pallete pallete, uint8_t ly)
{
	for (int i = 0; i < 8; i++) {
		uint8_t pix = row[i];
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
		for (int j = 0; j < SCALE; j++) {
			for (int k = 0; k < SCALE; k++) {
				uint16_t y = (ly) * SCALE + j;
				uint16_t x = (i + xpix) * SCALE + k;
				uint32_t coord = y * SCREEN_WIDTH * SCALE + x;
				if (x >= SCREEN_WIDTH * SCALE) continue;
				if (coord > SCREEN_WIDTH * SCALE * SCREEN_HEIGHT * SCALE) continue;
				ppu->fb[coord] = color;
			}
		}
	}
}

void
tile_xflip_row(uint8_t *row)
{
	for (int i = 0; i < 4; i++) {
		row[i] ^= row[7-i];
		row[7-i] ^= row[i];
		row[i] ^= row[7-i];
	}
}

void
sprite_yflip_row(uint8_t *r1, uint8_t *r2)
{
	for (int i = 0; i < 8; i++) {
		r1[i] ^= r2[i];
		r2[i] ^= r1[i];
		r1[i] ^= r2[i];
	}
}


void
sprite_render_row(struct PPU *ppu, struct Sprite *s, uint8_t ly)
{
	if (ppu->lcdc.obj_size)
		s->tile_id &= 0xfe;

	uint8_t row = ly - s->y + 16;
	assert(row < 16);
	uint8_t *t1 = get_tile_row(ppu, s->tile_id, row % 8, SPRITE);

	if (ppu->lcdc.obj_size)
		s->tile_id |= 0x01;

	uint8_t *t2 = get_tile_row(ppu, s->tile_id, 7 - (row % 8), SPRITE);


	if (s->yflip) {
		sprite_yflip_row(t1, t2);
	}

	if (s->xflip) {
		tile_xflip_row(t1);
	}

	s->y -= 16;
	s->x -= 8;

	if (row >= 8) {
		draw_tile_row(ppu, t2, s->x, s->dmg_palette ? OBP1 : OBP0, ly);
	} else {
		draw_tile_row(ppu, t1, s->x, s->dmg_palette ? OBP1 : OBP0, ly);
	}
	free(t1);
	free(t2);
}

void
render_bg_row(struct PPU *ppu, uint8_t *row, uint8_t ly)
{
	uint8_t scy = mem_read(ppu->mem, SCY);
	uint8_t scx = mem_read(ppu->mem, SCX);

	// for (int i = 0; i < LCD_WIDTH_TILES; i++) {
	// 	uint8_t *pix = get_tile_row(ppu, row[i], (ly + scy) % 8, WINDOW);
	// 	draw_tile_row(ppu, pix, i * 8, BGP, ly);
	// 	free(pix);
	// }

	for (int i = 0; i < LCD_WIDTH_TILES + 1; i++) {
		uint8_t *pix = get_tile_row(ppu, row[i], (ly + scy) % 8, WINDOW);
		draw_tile_row(ppu, pix, i * 8 - scx % 8, BGP, ly);
		free(pix);
	}
}

void
render_window_row(struct PPU *ppu, uint8_t *row, uint8_t ly)
{
	uint8_t wy = mem_read(ppu->mem, WY);
	uint8_t wx = mem_read(ppu->mem, WX);

	if (wy > ly)
		return;
	if (wx - 7> SCREEN_WIDTH) return;

	for (int i = 0; i < LCD_WIDTH_TILES; i++) {
		uint8_t *pix = get_tile_row(ppu, row[i], (ly - wy) % 8, WINDOW);
		draw_tile_row(ppu, pix, i * 8 + wx - 7, BGP, ly);
		free(pix);
	}
	wly++;
}

void
write_lcdc(struct PPU *ppu, struct LCD_Control *lcdc)
{
	uint8_t new = 0;
	new |= lcdc->enable << 7;
	new |= lcdc->w_tmap << 6;
	new |= lcdc->wenable << 5;
	new |= lcdc->tdata << 4;
	new |= lcdc->bg_tmap << 3;
	new |= lcdc->obj_size << 2;
	new |= lcdc->obj_enable << 1;
	new |= lcdc->bgwin_enable;
	mem_write(ppu->mem, LCDC, new);
}

struct Sprite **
oam_scan(struct PPU *ppu, uint8_t ly)
{
	struct Sprite **list = calloc(OAM_SPRITE_LIMIT, sizeof(struct Sprite *));

	int i = 0;
	for (int j = 0; j < 40 && i < OAM_SPRITE_LIMIT; j++) {
		struct Sprite *s = get_sprite(ppu, j);
		if (s->x > 0 && ly + 16 >= s->y
				&& ly + 16 < s->y + (ppu->lcdc.obj_size ? 16 : 8)) {
			list[i++] = s;
			continue;
		}

		free(s);
		s = NULL;
	}

	for (; i < OAM_SPRITE_LIMIT; i++) {
		list[i] = NULL;
	}

	return list;
}

void
ppu_draw(struct PPU *ppu, struct Sprite **list)
{
	uint8_t ly = mem_read(ppu->mem, LY);

	uint16_t adr = 0x9800;
	if (ppu->lcdc.bg_tmap)
		adr = 0x9C00;

	if (ppu->lcdc.bgwin_enable) {
		uint8_t *row = get_bg_row(ppu, adr, ly);
		render_bg_row(ppu, row, ly);
		free(row);
	}

	adr = 0x9800;
	if (ppu->lcdc.w_tmap)
		adr = 0x9C00;
	if (ppu->lcdc.wenable) {
		uint8_t *row = get_window_row(ppu, adr, ly);
		render_window_row(ppu, row, ly);
		if (row != NULL)
			free(row);
	}

	for (int i = 0; i < OAM_SPRITE_LIMIT; i++) {
		if (list[i] == NULL)
			continue;

		if (ppu->lcdc.obj_enable)
			sprite_render_row(ppu, list[i], ly);

		free(list[i]);
	}
	free(list);
}

struct Sprite **list = NULL;
void
ppu_run(struct PPU *ppu, int cycles)
{
	uint8_t ly = mem_read(ppu->mem, LY);
	uint8_t lyc = mem_read(ppu->mem, LYC);
	ppu->lcdc = read_lcdc(ppu);

	if (!ppu->lcdc.enable)
		return;

	ppu->tcycles += cycles * 4;
	for (int i = 0; i < cycles * 4; i++, ppu->tcycles++) {
		set_ppu_mode(ppu, ppu->mode.mode);
		switch (ppu->mode.mode) {
			case OAM_SCAN:
				if (ppu->tcycles >= 80) {
					list = oam_scan(ppu, ly);
					set_ppu_mode(ppu, DRAW);
					ppu->lcdc = read_lcdc(ppu);
				}
				break;
			case DRAW:
				if (ppu->tcycles >= 80 + 289) {
					ppu_draw(ppu, list);
					set_ppu_mode(ppu, HBLANK);
				}
				break;
			case HBLANK:
				if (ppu->tcycles < 456) {
					break;
				}

				if (ly >= 143) {
					set_ppu_mode(ppu, VBLANK);
					request_interrupt(ppu->mem, INTERRUPT_VBLANK);
					SDL_UpdateWindowSurface(ppu->win);
	 			} else {
					set_ppu_mode(ppu, OAM_SCAN);
					mem_write(ppu->mem, LY, ly + 1);
				}

				ppu->tcycles = 0;
				break;
			case VBLANK:
				wly = 0;
				if (ppu->tcycles < 456)
					break;

				ppu->tcycles = 0;
				if (ly >= 153) {
					ppu->tcycles = 0;
					ly = 0;
					mem_write(ppu->mem, LY, 0);
					set_ppu_mode(ppu, OAM_SCAN);
				} else {
					mem_write(ppu->mem, LY, ly + 1);
				}
				break;
		}

		request_stat(ppu);

		if (ly == lyc) {
			mem_write(ppu->mem, STAT, mem_read(ppu->mem, STAT) | LYC_LC);
		} else {
			mem_write(ppu->mem, STAT, mem_read(ppu->mem, STAT) & ~LYC_LC);
		}
	}
}

void
ppu_log(struct PPU *ppu)
{
	fprintf(ppu->log, "CYC: %05d ", ppu->tcycles);
	fprintf(ppu->log, "LCDC: %08b ", mem_read(ppu->mem, LCDC));
	fprintf(ppu->log, "LY: %02x ", mem_read(ppu->mem, LY));
	fprintf(ppu->log, "LYC: %02x ", mem_read(ppu->mem, LYC));
	fprintf(ppu->log, "STAT: %07b ", mem_read(ppu->mem, STAT));
	fprintf(ppu->log, "SCX: %02x ", mem_read(ppu->mem, SCX));
	fprintf(ppu->log, "SCY: %02x ", mem_read(ppu->mem, SCY));
	fprintf(ppu->log, "WX: %02x ", mem_read(ppu->mem, WX));
	fprintf(ppu->log, "WY: %02x ", mem_read(ppu->mem, WY));
	fprintf(ppu->log, "SL: %02x ", statline);
	fprintf(ppu->log, "\n");
}

