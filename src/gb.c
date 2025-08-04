#include <stdbool.h>
#include <stdlib.h>
#include "cpu.h"
#include "graphics.h"
#include "gb.h"
#include "mem.h"

struct GB *
gb_init(void)
{
	struct GB *gb = calloc(1, sizeof(struct GB));
	gb->cpu = init_cpu(gb->mem);
	if (gb->cpu == NULL) return NULL;

	gb->ppu = graphics_init(gb->mem);
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
		// graphics_scanline(gb->ppu);

		if (mem_read(gb->mem, gb->cpu->pc) == 0x00) {
			for (int i = 0x9880; i < 0x9c00; i++) {
				int c = mem_read(gb->mem, i);
				if (c != 0) {
					printf("%d\t", c);
				}
			}
			break;
		}
	}

	graphics_scanline(gb->ppu);
}
