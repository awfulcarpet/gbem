#include <stdbool.h>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

#include "cpu.h"
#include "mem.h"
#include "opcode.h"
#include "timer.h"
#include "ppu.h"
#include "gb.h"

int
main(int argc, char **argv) {
	struct GB * gb = gb_init();
	gb->cpu->pc = 0;

	if (gb == NULL) return 1;

	if (argc < 2) {
		fprintf(stderr, "usage: gbem <gb file>\n");
		return 1;
	}

	if (load_rom(gb->mem, argv[1])) {
		return 1;
	}

	gb_run(gb);
	return 0;
}
