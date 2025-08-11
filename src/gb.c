#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include "cpu.h"
#include "ppu.h"
#include "gb.h"
#include "mem.h"
#include "joypad.h"

double
getmsec() {
       struct timeval time;
       gettimeofday(&time, NULL);
       return (double)time.tv_sec * 1000 + (time.tv_usec/1000.0);
}

struct GB *
gb_init(void)
{
	struct GB *gb = calloc(1, sizeof(struct GB));
	gb->cpu = init_cpu(gb->mem);
	if (gb->cpu == NULL) return NULL;

	gb->ppu = ppu_init(gb->mem);
	if (gb->ppu == NULL) return NULL;

	gb->cpu->pc = 0x100;

	gb->running = 1;
	mem_write(gb->mem, JOYP, 0xcf);

	return gb;
}

void
gb_run(struct GB *gb)
{
	int cycles = 0;
	double time = getmsec();
	double delta = 0;

	double mcyc_hz = 4194304.0 / 4.0;

	while (gb->running) {
		delta = getmsec() - time;

		int cyc = mcyc_hz * delta/1000.0;

		get_input(gb);
		for (int i = 0; i < cyc; i += cycles) {
			cycles = execute(gb->cpu);
			ppu_run(gb->ppu, cycles);
		}


		if (delta < 16.74) {
			continue;
		}

		debug_draw(gb->ppu);
		SDL_UpdateWindowSurface(gb->ppu->win);
		SDL_UpdateWindowSurface(gb->ppu->debug_bgwin);
		SDL_UpdateWindowSurface(gb->ppu->debug_wwin);
		SDL_UpdateWindowSurface(gb->ppu->debug_owin);

		time = getmsec();
	}
}
