#include <SDL2/SDL_video.h>
#include <stdint.h>

#define SCALE 1

enum {
	SCREEN_WIDTH = 160,
	SCREEN_HEIGHT = 144,
};

enum {
	LCDC = 0xff40,
	STAT = 0xff41,
	LY = 0xff44,
	LYC = 0xff45,

	OAM = 0xFE00,

	SCY = 0xff42,
	SCX = 0xff43,

	WY = 0xff4a,
	WX = 0xff4b,

	VRAM = 0x8000,
};

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


enum {
	BYTES_PER_SPRITE = 4,
	BYTES_PER_TILE = 16,
};

enum {
	WINDOW_WIDTH_TILES = 32,
	WINDOW_HEIGHT_TILES = 32,
};


#define OAM_SPRITE_LIMIT 10

enum PPU_MODE {
	HBLANK = 0,
	VBLANK = 1,
	OAM_SCAN = 2,
	DRAW = 3,
};

struct LCD_Control {
	uint8_t enable:1;
	uint8_t w_tmap:1;
	uint8_t wenable:1;
	uint8_t tdata:1;
	uint8_t bg_tmap:1;
	uint8_t obj_size:1;
	uint8_t obj_enable:1;
	uint8_t bgwin_enable:1;
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


struct Mode {
	enum PPU_MODE mode;
	uint16_t dur; /* in dots */
};

struct PPU {
	struct Mode mode;

	uint8_t *mem;

	SDL_Window *win;
	uint32_t *fb;

	FILE *log;
};


struct PPU *ppu_init(uint8_t *mem);
int ppu_scanline(struct PPU *ppu);
int ppu_drawscreen(struct PPU *ppu);
void ppu_log(struct PPU *ppu);
