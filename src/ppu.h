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

	SCX = 0xff42,
	SCY = 0xff43,

	WX = 0xff4a,
	WY = 0xff4b,

};

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
int ppu_drawscreen(struct PPU *ppu);
void ppu_log(struct PPU *ppu);
