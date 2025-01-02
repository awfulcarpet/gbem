#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "opcode.h"

struct CPU *
init_cpu(void) {
	struct CPU *cpu = calloc(1, sizeof(struct CPU));
	cpu->a = 0;
	cpu->b = 0;
	cpu->c = 0;
	cpu->d = 0;
	cpu->e = 0;
	cpu->h = 0;
	cpu->l = 0;
	cpu->f.flags = 0;
	cpu->pc = 0;
	cpu->sp = 0;
	cpu->ie = 0;
	cpu->ir = 0;

	cpu->mcycles = 0;

	memset(cpu->memory, 0, 0xFFFF + 1);

	return cpu;
}

void
print_cpu_state(struct CPU *cpu)
{
	printf("CYC: %d ", cpu->mcycles);

	printf("%c%c%c%c ",
		cpu->f.z ? 'z' : '-', cpu->f.n ? 'n' : '-', cpu->f.h ? 'h' : '-', cpu->f.c ? 'c' : '-');

	printf("AF: %02x%02x BC: %02x%02x DE: %02x%02x HL: %02x%02x SP: %04x PC: %04x ",
		cpu->a, cpu->f.flags, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp, cpu->pc);

	printf("[HL]: %02x Stk: %02x %02x %02x %02x ",
		cpu->memory[cpu->h << 8 | cpu->l], cpu->memory[cpu->sp], cpu->memory[cpu->sp+1], cpu->memory[cpu->sp+2], cpu->memory[cpu->sp+3]);

	printf("nxt: %02x %02x %02x %02x ", cpu->memory[cpu->pc], cpu->memory[cpu->pc+1], cpu->memory[cpu->pc+2], cpu->memory[cpu->pc+3]);

	printf("\n");
}

static void
set_zn(struct CPU *cpu, uint8_t reg, uint8_t n)
{
	cpu->f.n = n;
	cpu->f.z = (reg == 0);
}

static int
inc_r16(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16(0b00110000, 4)

	reg++;

	set_r8_from_r16()

	return 2;
}

static int
dec_r16(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16(0b00110000, 4)

	reg--;

	set_r8_from_r16()

	return 2;
}

static int
add_hl_r16(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16(0b00110000, 4)
	uint16_t hl = cpu->h << 8 | cpu->l;
	/* TODO: do without uint32_t? */
	uint32_t res = hl + reg;

	cpu->f.n = 0;
	cpu->f.h = (((hl & 0xfff) + (reg & 0xfff)) & 0x1000) == 0x1000;
	cpu->f.c = res > 0xffff;

	cpu->h = res >> 8;
	cpu->l = res & 0xff;

	return 2;
}


static int
dec_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b00111000, 3)

	(*reg)--;

	set_zn(cpu, *reg, 1);
	cpu->f.h = (*reg & 0b1111) == 0b1111;

	if (((opcode & 0b00111000) >> 3) == m)
		return 3;
	return 1;
}

static int
inc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b00111000, 3)

	(*reg)++;

	set_zn(cpu, *reg, 0);
	/* TODO: possible bug? */
	cpu->f.h = (*reg & 0b1111) == 0b0000;

	if (((opcode & 0b00111000) >> 3) == m)
		return 3;
	return 1;
}

static int
ld_r16_imm16(struct CPU *cpu, uint8_t opcode)
{

	set_regs_r16(0b00110000, 4)
	reg = cpu->memory[cpu->pc + 1] << 8 | cpu->memory[cpu->pc];


	set_r8_from_r16()
	cpu->pc += 2;

	return 3;
}

static int
ld_r16mem_a(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16mem(0b00110000, 4)

	cpu->memory[reg] = cpu->a;

	if (op == hli)
		inc_r16(cpu, 0b00100000);
	if (op == hld)
		dec_r16(cpu, 0b00100000);

	return 2;
}

static int
ld_a_r16mem(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16mem(0b00110000, 4)

	cpu->a = cpu->memory[reg];

	if (op == hli)
		inc_r16(cpu, 0b00100000);
	if (op == hld)
		dec_r16(cpu, 0b00100000);
	return 2;
}

static int
ld_imm16_sp(struct CPU *cpu, uint8_t opcode)
{
	uint16_t adr = cpu->memory[cpu->pc + 1] << 8 | cpu->memory[cpu->pc];
	cpu->memory[adr] = cpu->sp & 0xff;
	cpu->memory[adr + 1] = cpu->sp >> 8;
	cpu->pc += 2;
	return 5;
}

static int
ld_r8_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *dst = NULL;
	uint8_t *src = NULL;
	set_regs_r8(dst, 0b00111000, 3);
	set_regs_r8(src, 0b00000111, 0);

	*dst = *src;

	if ((opcode & 0b00111000) >> 3 == m)
		return 2;
	if ((opcode & 0b00000111) == m)
		return 2;
	return 1;
}

static int
ld_r8_imm8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *dst = NULL;
	set_regs_r8(dst, 0b00111000, 3);

	*dst = cpu->memory[cpu->pc];

	cpu->pc += 1;

	if ((opcode & 0b00111000) >> 3 == m)
		return 3;

	return 2;
}

static void
rlca(struct CPU *cpu)
{
	cpu->f.c = cpu->a >> 7;
	cpu->a <<= 1;
	cpu->a |= cpu->f.c;

	cpu->f.z = 0;
	cpu->f.n = 0;
	cpu->f.h = 0;
}

static void
rrca(struct CPU *cpu)
{
	cpu->f.c = cpu->a & 0x01;
	cpu->a >>= 1;
	cpu->a |= cpu->f.c << 7;

	cpu->f.z = 0;
	cpu->f.n = 0;
	cpu->f.h = 0;
}

static void
rla(struct CPU *cpu)
{
	uint8_t tmp = cpu->a;
	cpu->a <<= 1;
	cpu->a |= cpu->f.c;
	cpu->f.c = tmp >> 7;

	cpu->f.z = 0;
	cpu->f.n = 0;
	cpu->f.h = 0;
}

static void
rra(struct CPU *cpu)
{
	uint8_t tmp = cpu->a;
	cpu->a >>= 1;
	cpu->a |= cpu->f.c << 7;
	cpu->f.c = tmp & 0x01;

	cpu->f.z = 0;
	cpu->f.n = 0;
	cpu->f.h = 0;
}

/* TODO: might be broken, manually test later */
static void
daa(struct CPU *cpu)
{
	uint8_t adj = 0;

	if (cpu->f.h || (!cpu->f.n && (cpu->a & 0x0F) > 0x09)) {
		adj |= 0x06;
	}

	if (cpu->f.c || (!cpu->f.n && cpu->a > 0x99)) {
		adj |= 0x60;
		cpu->f.c = 1;
	}

	cpu->a += adj * (cpu->f.n ? -1 : 1);

	cpu->f.z = (cpu->a == 0);
	cpu->f.h = 0;
}

static void
cpl(struct CPU *cpu)
{
	cpu->a = ~cpu->a;
	cpu->f.n = 1;
	cpu->f.h = 1;
}

static void
scf(struct CPU *cpu)
{
	cpu->f.c = 1;
	cpu->f.n = 0;
	cpu->f.h = 0;
}

static void
ccf(struct CPU *cpu)
{
	cpu->f.c = ~cpu->f.c;
	cpu->f.n = 0;
	cpu->f.h = 0;
}

static int
bit_shift(struct CPU *cpu, uint8_t opcode)
{
	switch (opcode >> 3) {
		case 0:
			rlca(cpu);
			break;
		case 1:
			rrca(cpu);
			break;
		case 2:
			rla(cpu);
			break;
		case 3:
			rra(cpu);
			break;
		case 4:
			daa(cpu);
			break;
		case 5:
			cpl(cpu);
			break;
		case 6:
			scf(cpu);
			break;
		case 7:
			ccf(cpu);
			break;
		default:
			unimlemented_opcode(opcode);
			break;
	}
	return 1;
}

int
jr_imm8(struct CPU *cpu, uint8_t opcode)
{
	/* jr e8 */
	if (opcode == 0x18)
		goto jmp;

	/* other */
	switch (opcode >> 3 & 0b11) {
		case nzero:
			if (cpu->f.z == 0)
				goto jmp;
			break;
		case zero:
			if (cpu->f.z)
				goto jmp;
			break;
		case ncarry:
			if (cpu->f.c == 0)
				goto jmp;
			break;
		case carry:
			if (cpu->f.c)
				goto jmp;
			break;
		default:
			fprintf(stderr, "incorrect jmp\n");
			exit(1);
		break;
	}

	cpu->pc++;
	return 2;

jmp:
	cpu->pc += (int8_t)cpu->memory[cpu->pc] + 1;
	return 3;
}

/* MASSIVE TODO: implement more complex stop based on outside hardware */
static int
stop(struct CPU *cpu)
{
	cpu->stop = 1;
	return 1;
}

/* TODO: implement 1 cycle delay later */
static int
ei(struct CPU *cpu)
{
	cpu->ie = 1;
	return 1;
}

static int
di(struct CPU *cpu)
{
	cpu->ie = 0;
	return 1;
}

static int
halt(struct CPU *cpu)
{
	cpu->halt = 1;
	return 1;
}

/* TODO? fix h flag */
static int
add_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0)

	cpu->f.c = (cpu->a + *reg) > 0xff;

	cpu->f.h = (cpu->a & 0b1111) + (*reg & 0b1111) >= 0b10000;
	cpu->f.n = 0;

	cpu->a += *reg;
	cpu->f.z = cpu->a == 0;

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
adc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0);

	uint16_t ans = cpu->a + *reg + cpu->f.c;
	uint8_t half_ans = (cpu->a & 0xf) + (*reg & 0xf) + cpu->f.c;

	cpu->a = ans;

	cpu->f.z = (cpu->a == 0);
	cpu->f.c = ans > 0xff;
	cpu->f.n = 0;
	cpu->f.h = half_ans > 0xf;

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
sub_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0);

	cpu->f.c = *reg > cpu->a;
	cpu->f.n = 1;
	cpu->f.h = (cpu->a & 0xf) < (*reg & 0xf);


	cpu->a -= *reg;
	cpu->f.z = (cpu->a == 0);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
sbc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0);
	uint8_t res = cpu->a - *reg - cpu->f.c;

	cpu->f.h = (cpu->a & 0xf) < ((*reg & 0xf) + (cpu->f.c));
	cpu->f.c = *reg + cpu->f.c > cpu->a;
	cpu->f.n = 1;

	cpu->a = res;
	cpu->f.z = (cpu->a == 0);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
and_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0)

	cpu->a &= *reg;

	cpu->f.z = (cpu->a == 0);
	cpu->f.n = 0;
	cpu->f.h = 1;
	cpu->f.c = 0;

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
xor_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0)

	cpu->a ^= *reg;

	cpu->f.z = (cpu->a == 0);
	cpu->f.n = 0;
	cpu->f.h = 0;
	cpu->f.c = 0;

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
or_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0)

	cpu->a |= *reg;

	cpu->f.z = (cpu->a == 0);
	cpu->f.n = 0;
	cpu->f.h = 0;
	cpu->f.c = 0;

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
cp_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = NULL;
	set_regs_r8(reg, 0b111, 0)
	uint8_t res = cpu->a - *reg;

	cpu->f.z = (res == 0);
	cpu->f.n = 1;
	cpu->f.h = (cpu->a & 0xf) < (*reg & 0xf);
	cpu->f.c = cpu->a < *reg;

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

int
execute(struct CPU *cpu) {
	uint8_t *opcode = &cpu->memory[cpu->pc];

	if (!cpu->halt && !cpu->stop)
		cpu->pc++;

	/* block 0 opcodes */
	if (*opcode <= 0x3f && *opcode >= 0x00) {

		/* NOP */
		if (*opcode == 0x00)
			return 1;


		/* ld r16,imm16 */
		if ((*opcode & 0b1111) == 0b0001) {
			return ld_r16_imm16(cpu, *opcode);
		}

		/* ld [r16mem],a */
		if ((*opcode & 0b1111) == 0b0010) {
			return ld_r16mem_a(cpu, *opcode);
		}

		/* ld a,[r16mem] */
		if ((*opcode & 0b1111) == 0b1010) {
			return ld_a_r16mem(cpu, *opcode);
		}

		/* ld [imm16], sp */
		if (*opcode == 0b00001000) {
			return ld_imm16_sp(cpu, *opcode);
		}

		/* inc r16 */
		if ((*opcode & 0b1111) == 0b0011) {
			return inc_r16(cpu, *opcode);
		}

		/* dec r16 */
		if ((*opcode & 0b1111) == 0b1011) {
			return dec_r16(cpu, *opcode);
		}

		/* add hl, r16 */
		if ((*opcode & 0b1111) == 0b1001) {
			return add_hl_r16(cpu, *opcode);
		}

		/* inc r8 */
		if ((*opcode & 0b111) == 0b100) {
			return inc_r8(cpu, *opcode);
		}

		/* dec r8 */
		if ((*opcode & 0b111) == 0b101) {
			return dec_r8(cpu, *opcode);
		}

		/* ld r8 imm8 */
		if ((*opcode & 0b111) == 0b110) {
			return ld_r8_imm8(cpu, *opcode);
		}

		/* bit shifts */
		if ((*opcode & 0b111) == 0b111) {
			return bit_shift(cpu, *opcode);
		}

		/* jr e8 */
		/* jr cond, e8 */
		if (*opcode == 0b00011000 || (*opcode & 0b00100111) == 0b00100000) {
			return jr_imm8(cpu, *opcode);
		}

		/* stop */
		if (*opcode == 0x10)
			return stop(cpu);

		unimlemented_opcode(*opcode);
	}

	/* block 1 */
	if (*opcode >= 0x40 && *opcode <= 0x7F) {

		/* halt */
		/* TODO: implement waking from halt*/
		if (*opcode == 0x76) {
			return halt(cpu);
		}

		return ld_r8_r8(cpu, *opcode);
	}

	/* block 2 */
	if (*opcode >= 0x80 && *opcode <= 0xbf) {
		uint8_t op = *opcode >> 3 & 0b111;

		if (op == 0b000) {
			return add_r8(cpu, *opcode);
		}

		if (op == 0b001) {
			return adc_r8(cpu, *opcode);
		}

		if (op == 0b010) {
			return sub_r8(cpu, *opcode);
		}

		if (op == 0b011) {
			return sbc_r8(cpu, *opcode);
		}

		if (op == 0b100) {
			return and_r8(cpu, *opcode);
		}

		if (op == 0b101) {
			return xor_r8(cpu, *opcode);
		}

		if (op == 0b110) {
			return or_r8(cpu, *opcode);
		}

		if (op == 0b111) {
			return cp_r8(cpu, *opcode);
		}

		unimlemented_opcode(*opcode);
	}

	/* block 3 */

	if (*opcode == 0xf3)
		return ei(cpu);
	if (*opcode == 0xfb)
		return di(cpu);

	unimlemented_opcode(*opcode);

	return 1;
}
