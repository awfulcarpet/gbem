enum {
	DIV = 0xFF04,
	TIMA = 0xFF05,
	TMA = 0xFF06,
	TAC = 0xFF07,
};

enum {
	TAC_CLOCK = 1 << 1 | 1,
	TAC_ENABLE = 1 << 2,
};

void timer_incr(struct CPU *cpu, int cycles);
