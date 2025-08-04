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
};

enum PPU_MODE {
	HBLANK = 0,
	VBLANK = 1,
	OAM_SCAN = 2,
	DRAW = 3,
};

struct Mode {
	enum PPU_MODE mode;
	uint16_t dur; /* in dots */
};

struct PPU {
	struct Mode mode;
	uint8_t ly;

	uint8_t *mem;

	SDL_Window *win;
	uint32_t *fb;
};


struct PPU *graphics_init(uint8_t *mem);
int graphics_scanline(struct PPU *ppu);
