#include <stdbool.h>
#include <stdlib.h>
#include "cpu.h"
#include "graphics.h"
#include "gb.h"

struct GB *
gb_init(void)
{
	struct GB *gb = calloc(1, sizeof(struct GB));
	gb->cpu = init_cpu();
	if (gb->cpu == NULL) return NULL;

	gb->ppu = graphics_init();
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
		graphics_scanline(gb->ppu);
	}
}
