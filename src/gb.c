#include <stdbool.h>
#include <stdlib.h>
#include "cpu.h"
#include "ppu.h"
#include "gb.h"
#include "mem.h"

struct GB *
gb_init(void)
{
	struct GB *gb = calloc(1, sizeof(struct GB));
	gb->cpu = init_cpu(gb->mem);
	if (gb->cpu == NULL) return NULL;

	gb->ppu = ppu_init(gb->mem);
	if (gb->ppu == NULL) return NULL;

	gb->cpu->pc = 0x100;

	return gb;
}

void
gb_run(struct GB *gb)
{
	while (true) {
		print_cpu_state(gb->cpu);
		execute(gb->cpu);
		ppu_log(gb->ppu);
		if (mem_read(gb->mem, gb->cpu->pc) == 0x00) {
			break;
		}
	}

	ppu_drawscreen(gb->ppu);
	for (int i = 0; i < SCREEN_HEIGHT; i++) {
	ppu_scanline(gb->ppu);
	}
}
