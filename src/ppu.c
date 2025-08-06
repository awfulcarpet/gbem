#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <assert.h>
#include <stdint.h>
#include "ppu.h"
#include "mem.h"

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
get_tile_row(struct PPU *ppu, uint8_t id, uint8_t row)
{
	assert(row < 8);
	uint8_t *pix = calloc(8, sizeof(uint8_t));

	uint8_t h = 0, l = 0;
	uint16_t adr = VRAM + id * BYTES_PER_TILE;

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

struct Tile *
get_tile(struct PPU *ppu, uint8_t id)
{
	struct Tile *t = calloc(1, sizeof(struct Tile));
	if (t == NULL)
		return NULL;
	t->id = id;

	/* https://gbdev.io/pandocs/Tile_Data.html#data-format */
	uint8_t h = 0, l = 0;
	uint16_t adr = VRAM + id * BYTES_PER_TILE;

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

uint8_t *
get_window_row(struct PPU *ppu, uint16_t adr, uint8_t ly)
{
	uint8_t *row = calloc(20, sizeof(uint8_t *));
	uint8_t scy = mem_read(ppu->mem, SCY);

	for (int i = 0; i < LCD_WIDTH_TILES; i++) {
		row[i] = mem_read(ppu->mem, adr + (ly + scy)/8 * WINDOW_WIDTH_TILES + i);
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

struct PPU *
ppu_init(uint8_t *mem)
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

void
draw_tile_row(struct PPU *ppu, uint8_t *row, uint8_t xpix, enum Pallete pallete, uint8_t ly)
{
	for (int j = 0; j < 8; j++) {
		uint8_t pix = row[j];
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
		uint16_t coord = ly * SCREEN_WIDTH + j + xpix;
		if (coord > SCREEN_WIDTH * SCREEN_HEIGHT) continue;
		ppu->fb[coord] = color;
	}
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
			uint16_t coord = ypix * SCREEN_WIDTH + i * SCREEN_WIDTH + j + xpix;
			if (coord > SCREEN_WIDTH * SCREEN_HEIGHT) continue;
			ppu->fb[coord] = color;
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
sprite_yflip_row(uint8_t *r1, uint8_t *r2)
{
	for (int i = 0; i < 8; i++) {
		r1[i] ^= r2[i];
		r2[i] ^= r1[i];
		r1[i] ^= r2[i];
	}
}

void
sprite_yflip(struct Tile *t1, struct Tile *t2)
{
	if (t2 == NULL) {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 8; j++) {
				t1->pixels[i][j] ^= t1->pixels[7-i][j];
				t1->pixels[7-i][j] ^= t1->pixels[i][j];
				t1->pixels[i][j] ^= t1->pixels[7-i][j];
			}
		}
		return;
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			t1->pixels[i][j] ^= t2->pixels[7-i][j];
			t2->pixels[7-i][j] ^= t1->pixels[i][j];
			t1->pixels[i][j] ^= t2->pixels[7-i][j];
		}
	}
}

void
sprite_render_row(struct PPU *ppu, struct Sprite *s, uint8_t ly)
{
	struct LCD_Control lcdc = read_lcdc(ppu);

	if (lcdc.obj_size)
		s->tile_id &= 0xfe;
	// uint8_t *t2 = get_tile_row(ppu, s->tile_id | 0x01, ly);

	uint8_t row = ly - s->y + 16;
	uint8_t *t1 = get_tile_row(ppu, s->tile_id, row);
	uint8_t *t2 = get_tile_row(ppu, s->tile_id, 7 - row);


	if (s->yflip) {
		sprite_yflip_row(t1, t2);
	}

	if (s->xflip) {
		tile_xflip_row(t1);
	}

	s->y -= 16;
	s->x -= 8;

	draw_tile_row(ppu, t1, s->x, s->dmg_palette ? OBP1 : OBP0, ly);
	free(t1);
	free(t2);
}

void
sprite_render(struct PPU *ppu, struct Sprite *s, uint8_t row)
{
	struct LCD_Control lcdc = read_lcdc(ppu);

	if (lcdc.obj_size)
		s->tile_id &= 0xfe;

	struct Tile *t1 = get_tile(ppu, s->tile_id);
	struct Tile *t2 = get_tile(ppu, s->tile_id | 0x01);

	for (int i = 0; i < 8; i++)
		// fprintf(stderr, "%c ", t1->pixels[row][i] == 0 ? '-' : t1->pixels[row][i] + '0');
		fprintf(stderr, "%3d ", t1->pixels[row][i]);
	fprintf(stderr, "\n");

	// if (s->xflip) {
	// 	tile_xflip(t1);
	// 	if (lcdc.obj_size)
	// 		tile_xflip(t2);
	// }
	//
	// if (s->yflip)
	// 	sprite_yflip(t1, lcdc.obj_size ? t2 : NULL);
	//
	// s->y -= 16;
	// s->x -= 8;
	// draw_tile(ppu, t1, s->x, s->y, s->dmg_palette ? OBP1 : OBP0);
	// if (lcdc.obj_size)
	// 	draw_tile(ppu, t2, s->x, s->y + 8, s->dmg_palette ? OBP1 : OBP0);

	free(t1);
	free(t2);
}

void
render_window_row(struct PPU *ppu, uint8_t *row, uint8_t ly)
{
	for (int i = 0; i < LCD_WIDTH_TILES; i++) {
		uint8_t *pix = get_tile_row(ppu, row[i], ly % 8);
		draw_tile_row(ppu, pix, i * 8, BGP, ly);
		free(pix);
	}
}

void
render_window(struct PPU *ppu, struct Window *win, uint8_t x, uint8_t y)
{
	uint8_t ly = mem_read(ppu->mem, LY);
	for (int i = 0; i < 18; i++) {
		for (int j = 0; j < 20; j++) {
			draw_tile(ppu, win->tiles[i][j], x + j * 8, y + i * 8, BGP);
			free(win->tiles[i][j]);
		}
	}
	free(win);
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
	struct LCD_Control lcdc = read_lcdc(ppu);

	int i = 0;
	for (int j = 0; j < 40 && i < OAM_SPRITE_LIMIT; j++) {
		struct Sprite *s = get_sprite(ppu, j);
		if (s->x > 0 && ly + 16 >= s->y
				&& ly + 16 < s->y + (lcdc.obj_size ? 16 : 8)) {
			list[i++] = s;
			continue;
		}

		free(s);
		s = NULL;
	}


	return list;
}

int
ppu_scanline(struct PPU *ppu)
{
	set_ppu_mode(ppu, OAM_SCAN);
	uint8_t ly = mem_read(ppu->mem, LY);
	struct LCD_Control lcdc = read_lcdc(ppu);

	if (ly >= 8 && ly <= 15)
		lcdc.bgwin_enable = 0;
	if (ly >= 13 * 8 && ly <= 14 * 8)
		lcdc.obj_enable = 0;

	uint16_t adr = 0x9800;
	if (lcdc.bg_tmap)
		adr = 0x9C00;

	uint8_t *row = get_window_row(ppu, adr, ly);
	if (!lcdc.bgwin_enable) {
		memset(row, 0, WINDOW_WIDTH_TILES);
	}
	render_window_row(ppu, row, ly);
	free(row);


	fprintf(ppu->log, "ly: %d| ", ly);
	struct Sprite **list = oam_scan(ppu, ly);
	for (int i = 0; i < OAM_SPRITE_LIMIT; i++) {
		if (list[i] == NULL)
			break;
		fprintf(ppu->log, "(%02x %d %d) ", list[i]->tile_id, list[i]->xflip, list[i]->yflip);

		if (lcdc.obj_enable)
			sprite_render_row(ppu, list[i], ly);
		free(list[i]);
	}
	free(list);
	fprintf(ppu->log, "\n");

	mem_write(ppu->mem, LY, ly + 1);
	SDL_UpdateWindowSurface(ppu->win);
	return 0;
}

void
ppu_log(struct PPU *ppu)
{
	fprintf(ppu->log, "LCDC: %08b ", mem_read(ppu->mem, LCDC));
	fprintf(ppu->log, "LY: %02x ", mem_read(ppu->mem, LY));
	fprintf(ppu->log, "LYC: %02x ", mem_read(ppu->mem, LYC));
	fprintf(ppu->log, "STAT: %07b ", mem_read(ppu->mem, STAT));
	fprintf(ppu->log, "\n");
}

