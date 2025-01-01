#include <stdint.h>

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

	uint8_t halt;
	uint8_t stop;

	uint8_t memory[0xFFFF + 1];
	uint32_t mcycles;
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

#define set_regs_r8(reg, mask, shift) \
switch ((opcode & mask) >> shift) { \
	case b: \
		reg = &cpu->b; \
		break; \
	case c: \
		reg = &cpu->c; \
		break; \
	case d: \
		reg = &cpu->d; \
		break; \
	case e: \
		reg = &cpu->e; \
		break; \
	case h: \
		reg = &cpu->h; \
		break; \
	case l: \
		reg = &cpu->l; \
		break; \
	case m: \
		reg = &cpu->memory[cpu->h << 8 | cpu->l]; \
		break; \
	case a: \
		reg = &cpu->a; \
		break; \
}; \

struct CPU *init_cpu(void);
int execute(struct CPU *cpu);
void print_cpu_state(struct CPU *cpu);
