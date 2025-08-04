struct GB {
	struct CPU *cpu;
	struct PPU *ppu;
};

struct GB * gb_init(void);
void gb_run(struct GB *gb);
