#include <stdint.h>
#include <stdio.h>

enum r16 {
	bc,
	de,
	hl,
	sp,
};

enum r16mem {
	hli = 2,
	hld = 3,
};

enum r16stck {
	s_bc,
	s_de,
	s_hl,
	s_af,
};

enum r8 {
	b,
	c,
	d,
	e,
	h,
	l,
	m, /* [hl] */
	a,
};

enum cond {
	nzero = 0,
	zero,
	ncarry,
	carry = 3,
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

enum {
	IE = 0xFFFF,
	IF = 0xFF0F,

	SB = 0xFF01,
	SC = 0xFF02,
};

enum INTERRUPT {
	INTERRUPT_VBLANK = 1 << 0,
	INTERRUPT_STAT    = 1 << 1,
	INTERRUPT_TIMER  = 1 << 2,
	INTERRUPT_SERIAL = 1 << 3,
	INTERRUPT_JOYPAD = 1 << 4,
};

enum IME_STATE {
	IME_NEXT = -1, /* have ime set on next cycle */
	IME_SET = 1,
	IME_UNSET = 0,
};


struct CPU {
	// registers
	uint8_t a;
	union Flags f;
	uint8_t b,c;
	uint8_t d,e;
	uint8_t h,l;
	uint16_t sp, pc;

	int8_t ime;

	uint8_t halt;
	uint8_t stop;

	uint8_t *memory;
	uint32_t mcycles;

	uint16_t div;

	FILE *log;
};


#define get_r16(h, l) \
	reg = (h << 8) | l; \
	high = &h; \
	low = &l; \

#define set_r8_from_r16() \
if (op == sp) { \
	cpu->sp = reg; \
} else { \
	*high = reg >> 8; \
	*low = reg & 0xff; \
} \

#define set_regs_r16mem(mask, shift) \
int op = (opcode & mask) >> shift; \
uint16_t reg = 0; \
uint8_t *high, *low; \
high = low = NULL; \
\
switch (op) { \
	case bc: \
		get_r16(cpu->b, cpu->c); \
		break; \
	case de: \
		get_r16(cpu->d, cpu->e); \
		break; \
	case hli: \
	case hld: \
		get_r16(cpu->h, cpu->l); \
		break; \
}; \

#define set_regs_r16(mask, shift) \
int op = (opcode & mask) >> shift; \
uint16_t reg = 0; \
uint8_t *high, *low; \
high = low = NULL; \
\
switch (op) { \
	case bc: \
		get_r16(cpu->b, cpu->c); \
		break; \
	case de: \
		get_r16(cpu->d, cpu->e); \
		break; \
	case hl: \
		get_r16(cpu->h, cpu->l); \
		break; \
	case sp: \
		reg = cpu->sp; \
		break; \
}; \

struct CPU *init_cpu(uint8_t *mem);
int execute(struct CPU *cpu);
int execute_opcode(struct CPU *cpu);
void cpu_log(struct CPU *cpu);
void print_cpu_state(struct CPU *cpu);
