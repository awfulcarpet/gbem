#include <stdint.h>

enum r16 {
	bc,
	de,
	hl,
	sp,
};

union Flags {
	struct {
		uint8_t pad:4;
		uint8_t c:1;
		uint8_t h:1;
		uint8_t n:1;
		uint8_t z:1;
	};
	uint8_t flags;
};

struct CPU {
	// registers
	uint8_t a;
	union Flags f;
	uint8_t b,c;
	uint8_t d,e;
	uint8_t h,l;
	uint16_t sp, pc;

	uint8_t ie; // interrupt enable
	uint8_t ir; // instruction register

	uint8_t memory[0xFFFF + 1];
	uint32_t mcycles;
};

#define get_r16(h, l) \
	reg = (h << 8) | l; \
	high = &h; \
	low = &l; \

struct CPU *init_cpu(void);
int execute(struct CPU *cpu);
void print_cpu_state(struct CPU *cpu);
