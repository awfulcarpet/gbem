#include <stdbool.h>
#include <stdint.h>
struct GB {
	struct CPU *cpu;
	struct PPU *ppu;

	bool running;

	uint8_t mem[1 << 16];
};

double getmsec();
struct GB * gb_init(void);
void gb_run(struct GB *gb);
