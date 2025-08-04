#include <stdint.h>
struct GB {
	struct CPU *cpu;
	struct PPU *ppu;

	uint8_t mem[1 << 16];
};

struct GB * gb_init(void);
void gb_run(struct GB *gb);
