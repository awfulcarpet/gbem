#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "mem.h"
#include "opcode.h"
#include "timer.h"
#include "ppu.h"


struct CPU *
init_cpu(uint8_t *mem) {
	struct CPU *cpu = calloc(1, sizeof(struct CPU));
	/* https://bgb.bircd.org/pandocs.htm#powerupsequence */
	cpu->pc = 0x101;

	cpu->a = 0x01;
	cpu->f.flags = 0xb0;

	cpu->b = 0x00;
	cpu->c = 0x13;

	cpu->d = 0x00;
	cpu->e = 0xd8;

	cpu->h = 0x01;
	cpu->l = 0x4d;

	cpu->sp = 0xFFFE;

	if (mem != NULL)
		cpu->memory = mem;

	cpu->log = fopen("cpu.log", "w");

	return cpu;
}

/* parse r8 used from lowest bit position */
static uint8_t *
parse_r8(struct CPU *cpu, uint8_t opcode, uint8_t bit)
{
	switch ((opcode >> bit) & 7) {
		case b:
			return &cpu->b;
			break;
		case c:
			return &cpu->c;
			break;
		case d:
			return &cpu->d;
			break;
		case e:
			return &cpu->e;
			break;
		case h:
			return &cpu->h;
			break;
		case l:
			return &cpu->l;
			break;
		case m:
			return &cpu->memory[cpu->h << 8 | cpu->l];
			break;
		case a:
			return &cpu->a;
			break;
		default: /* should be unreachable */
			assert(NULL);
			break;
	};
}


static void
set_hc(struct CPU *cpu, uint8_t a, uint8_t b)
{
	cpu->f.h = (((a & 0xF) + (b & 0xF)) & 0x10) == 0x10;
	cpu->f.c = (((a & 0xFF) + (b & 0xFF)) & 0x100) == 0x100;
}

static uint16_t
pop(struct CPU *cpu, uint8_t *h, uint8_t *l)
{
	cpu->sp += 2;
	if (h != NULL)
		*h = mem_read(cpu->memory, cpu->sp - 1);
	if (l != NULL)
		*l = mem_read(cpu->memory, cpu->sp - 2);

	return mem_read(cpu->memory, cpu->sp - 1) << 8 | mem_read(cpu->memory, cpu->sp - 2);
}

static void
push(struct CPU *cpu, uint8_t h, uint8_t l)
{
	mem_write(cpu->memory, --cpu->sp, h);
	mem_write(cpu->memory, --cpu->sp, l);
}


static void
add(struct CPU *cpu, uint8_t n)
{
	set_hc(cpu, cpu->a, n);
	cpu->f.n = 0;

	cpu->a += n;
	cpu->f.z = cpu->a == 0;
}

static void
adc(struct CPU *cpu, uint8_t n)
{
	uint16_t ans = cpu->a + n + cpu->f.c;
	uint8_t half_ans = (cpu->a & 0xf) + (n & 0xf) + cpu->f.c;

	cpu->a = ans;

	cpu->f.z = (cpu->a == 0);
	cpu->f.c = ans > 0xff;
	cpu->f.n = 0;
	cpu->f.h = half_ans > 0xf;
}

static void
sub(struct CPU *cpu, uint8_t n)
{
	cpu->f.c = n > cpu->a;
	cpu->f.n = 1;
	cpu->f.h = (cpu->a & 0xf) < (n & 0xf);


	cpu->a -= n;
	cpu->f.z = (cpu->a == 0);
}

static void
sbc(struct CPU *cpu, uint8_t n)
{
	uint8_t res = cpu->a - n - cpu->f.c;

	cpu->f.h = (cpu->a & 0xf) < ((n & 0xf) + (cpu->f.c));
	cpu->f.c = n + cpu->f.c > cpu->a;
	cpu->f.n = 1;

	cpu->a = res;
	cpu->f.z = (cpu->a == 0);
}

static void
and(struct CPU *cpu, uint8_t n)
{
	cpu->a &= n;

	cpu->f.z = (cpu->a == 0);
	cpu->f.n = 0;
	cpu->f.h = 1;
	cpu->f.c = 0;
}

static void
xor(struct CPU *cpu, uint8_t n)
{
	cpu->a ^= n;

	cpu->f.z = (cpu->a == 0);
	cpu->f.n = 0;
	cpu->f.h = 0;
	cpu->f.c = 0;
}

static void
or(struct CPU *cpu, uint8_t n)
{
	cpu->a |= n;

	cpu->f.z = (cpu->a == 0);
	cpu->f.n = 0;
	cpu->f.h = 0;
	cpu->f.c = 0;
}

static void
cp(struct CPU *cpu, uint8_t n)
{
	uint8_t res = cpu->a - n;

	cpu->f.z = (res == 0);
	cpu->f.n = 1;
	cpu->f.h = (cpu->a & 0xf) < (n & 0xf);
	cpu->f.c = cpu->a < n;
}

static void
set_zn(struct CPU *cpu, uint8_t reg, uint8_t n)
{
	cpu->f.n = n;
	cpu->f.z = (reg == 0);
}

static void
jp(struct CPU *cpu, uint8_t h, uint8_t l)
{
	cpu->pc = h << 8 | l;
}

static void
call(struct CPU *cpu, uint8_t h, uint8_t l)
{
	cpu->pc += 2;
	push(cpu, cpu->pc >> 8, cpu->pc & 0xff);

	cpu->pc = h << 8 | l;
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
	set_regs_r16(0b00110000, 4);
	(void)high;
	(void)low;

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
	uint8_t *reg = parse_r8(cpu, opcode, 3);

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
	uint8_t *reg = parse_r8(cpu, opcode, 3);

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

	reg = mem_read(cpu->memory, cpu->pc + 1) << 8 | mem_read(cpu->memory, cpu->pc);

	set_r8_from_r16()
	cpu->pc += 2;

	return 3;
}

static int
ld_r16mem_a(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16mem(0b00110000, 4);
	(void)high;
	(void)low;

	mem_write(cpu->memory, reg, cpu->a);

	if (op == hli)
		inc_r16(cpu, hl << 4);
	if (op == hld)
		dec_r16(cpu, hl << 4);

	return 2;
}

static int
ld_a_r16mem(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16mem(0b00110000, 4);
	(void)high;
	(void)low;

	cpu->a = mem_read(cpu->memory, reg);

	if (op == hli)
		inc_r16(cpu, hl << 4);
	if (op == hld)
		dec_r16(cpu, hl << 4);
	return 2;
}

static int
ld_imm16_sp(struct CPU *cpu)
{
	uint16_t adr = mem_read(cpu->memory, cpu->pc + 1) << 8 | mem_read(cpu->memory, cpu->pc);

	mem_write(cpu->memory, adr, cpu->sp & 0xff);
	mem_write(cpu->memory, adr + 1, cpu->sp >> 8);
	cpu->pc += 2;
	return 5;
}

static int
ld_r8_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *dst = parse_r8(cpu, opcode, 3);
	uint8_t *src = parse_r8(cpu, opcode, 0);

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
	uint8_t *dst = parse_r8(cpu, opcode, 3);

	*dst = mem_read(cpu->memory, cpu->pc);

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

static int
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
	cpu->pc += (int8_t)mem_read(cpu->memory, cpu->pc) + 1;
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
	cpu->ime = IME_NEXT;
	return 1;
}

static int
di(struct CPU *cpu)
{
	cpu->ime = IME_UNSET;
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
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	add(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
adc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	adc(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
sub_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	sub(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
sbc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	sbc(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
and_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	and(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
xor_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	xor(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
or_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	or(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

static int
cp_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	cp(cpu, *reg);

	if ((opcode & 0b111) == m)
		return 2;

	return 1;
}

/* TODO? fix h flag */
static int
add_imm8(struct CPU *cpu, uint8_t n)
{
	add(cpu, n);

	cpu->pc++;
	return 2;
}

static int
adc_imm8(struct CPU *cpu, uint8_t n)
{
	adc(cpu, n);

	cpu->pc++;
	return 2;
}

static int
sub_imm8(struct CPU *cpu, uint8_t n)
{
	sub(cpu, n);

	cpu->pc++;
	return 2;
}

static int
sbc_imm8(struct CPU *cpu, uint8_t n)
{
	sbc(cpu, n);

	cpu->pc++;
	return 2;
}

static int
and_imm8(struct CPU *cpu, uint8_t n)
{
	and(cpu, n);
	cpu->pc++;
	return 2;
}

static int
xor_imm8(struct CPU *cpu, uint8_t n)
{
	xor(cpu, n);
	cpu->pc++;
	return 2;
}

static int
or_imm8(struct CPU *cpu, uint8_t n)
{
	or(cpu, n);
	cpu->pc++;
	return 2;
}

static int
cp_imm8(struct CPU *cpu, uint8_t n)
{
	cp(cpu, n);
	cpu->pc++;
	return 2;
}

static int
ret(struct CPU *cpu)
{
	cpu->pc = pop(cpu, NULL, NULL);
	return 4;
}

static int
reti(struct CPU *cpu)
{
	ret(cpu);
	cpu->ime = 1;
	return 4;
}

static int
ret_cond(struct CPU *cpu, uint8_t opcode)
{
	switch (opcode >> 3 & 0b11) {
		case nzero:
			if (cpu->f.z == 0) goto ret;
			break;
		case zero:
			if (cpu->f.z == 1) goto ret;
			break;
		case ncarry:
			if (cpu->f.c == 0) goto ret;
			break;
		case carry:
			if (cpu->f.c == 1) goto ret;
			break;
		default:
			break;
	}

	return 2;

ret:
	ret(cpu);
	return 5;
}

static int
jp_a16(struct CPU *cpu)
{
	jp(cpu, mem_read(cpu->memory, cpu->pc + 1), mem_read(cpu->memory, cpu->pc));
	return 4;
}

static int
jp_hl(struct CPU *cpu)
{
	jp(cpu, cpu->h, cpu->l);
	return 1;
}

static int
jp_cond(struct CPU *cpu, uint8_t opcode)
{
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

	cpu->pc += 2;
	return 3;

jmp:
	jp(cpu, mem_read(cpu->memory, cpu->pc + 1), mem_read(cpu->memory, cpu->pc));
	return 4;
}

static int
call_a16(struct CPU *cpu)
{
	call(cpu, mem_read(cpu->memory, cpu->pc + 1), mem_read(cpu->memory, cpu->pc));
	return 6;
}

static int
call_cond(struct CPU *cpu, uint8_t opcode)
{
	switch (opcode >> 3 & 0b11) {
		case nzero:
			if (cpu->f.z == 0)
				goto call;
			break;
		case zero:
			if (cpu->f.z)
				goto call;
			break;
		case ncarry:
			if (cpu->f.c == 0)
				goto call;
			break;
		case carry:
			if (cpu->f.c)
				goto call;
			break;
		default:
			fprintf(stderr, "incorrect call\n");
			exit(1);
		break;
	}

	cpu->pc += 2;
	return 3;

call:
	call(cpu, mem_read(cpu->memory, cpu->pc + 1), mem_read(cpu->memory, cpu->pc));
	return 6;
}

static int
rst(struct CPU *cpu, uint8_t opcode)
{
	uint16_t adr = (opcode >> 3 & 0b111) * 8;
	push(cpu, cpu->pc >> 8, cpu->pc & 0xff);

	jp(cpu, adr >> 8, adr & 0xff);
	return 4;
}

static int
ld_sp_hl(struct CPU *cpu)
{
	cpu->sp = cpu->h << 8 | cpu->l;
	return 2;
}

static int
pop_r16stk(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16stk(0x0011000, 4);
	(void)reg;

	pop(cpu, high, low);

	if (op == s_af)
		*low &= 0xf0;
	return 3;
}

static int
push_r16stk(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16stk(0b0011000, 4);
	(void)reg;

	mem_write(cpu->memory, --cpu->sp, *high);
	mem_write(cpu->memory, --cpu->sp, *low);

	return 4;
}

static int
ldh(struct CPU *cpu, const uint8_t opcode)
{
	switch (opcode) {
		case 0xE0:
			mem_write(cpu->memory, 0xFF00 + mem_read(cpu->memory, cpu->pc++), cpu->a);
			return 3;
		break;
		case 0xE2:
			mem_write(cpu->memory, 0xFF00 + cpu->c, cpu->a);
			return 2;
		break;
		case 0xEA:
			mem_write(cpu->memory, mem_read(cpu->memory, cpu->pc) | mem_read(cpu->memory, cpu->pc + 1) << 8, cpu->a);
			cpu->pc += 2;
			return 4;
		break;
		case 0xf0:
			cpu->a = mem_read(cpu->memory, 0xFF00 + mem_read(cpu->memory, cpu->pc++));
			return 3;
		break;
		case 0xf2:
			cpu->a = mem_read(cpu->memory, 0xFF00 + cpu->c);
			return 2;
		break;
		case 0xf8: {
			uint8_t e = mem_read(cpu->memory, cpu->pc++);

			cpu->f.z = 0;
			cpu->f.n = 0;

			set_hc(cpu, e, cpu->sp);

			cpu->h = ((cpu->sp + (int8_t)e) & 0xFF00) >> 8;
			cpu->l = (cpu->sp + (int8_t)e) & 0xFF;
			return 3;
			break;
		}
		case 0xfa:
			cpu->a = mem_read(cpu->memory, mem_read(cpu->memory, cpu->pc) | mem_read(cpu->memory, cpu->pc + 1) << 8);
			cpu->pc += 2;
			return 4;
		break;
		default:
			unimlemented_opcode(opcode);
		break;
	}

	return 3;
}

static int
add_sp_imm8(struct CPU *cpu)
{
	uint8_t e = mem_read(cpu->memory, cpu->pc++);

	cpu->f.z = 0;
	cpu->f.n = 0;

	set_hc(cpu, e, cpu->sp);

	cpu->sp += (int8_t)e;

	return 4;
}

static int
rlc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);


	cpu->f.c = *reg >> 7;
	*reg <<= 1;
	*reg |= cpu->f.c;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x06)
		return 4;

	return 2;
}

static int
rrc_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);


	cpu->f.c = *reg & 1;
	*reg >>= 1;
	*reg |= cpu->f.c << 7;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x0e)
		return 4;

	return 2;
}

static int
rl_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	uint8_t tmp = *reg >> 7;

	*reg <<= 1;
	*reg |= cpu->f.c;
	cpu->f.c = tmp;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x16)
		return 4;

	return 2;
}

static int
rr_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	uint8_t tmp = *reg & 1;

	*reg >>= 1;
	*reg |= cpu->f.c << 7;
	cpu->f.c = tmp;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x1e)
		return 4;

	return 2;
}

static int
sla_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	cpu->f.c = *reg >> 7;
	*reg <<= 1;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x26)
		return 4;

	return 2;
}

static int
sra_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	cpu->f.c = *reg & 1;
	*reg >>= 1;
	*reg |= (*reg >> 6) << 7;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x2e)
		return 4;

	return 2;
}

static int
swap_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	/* TODO: XOR? */
	uint8_t tmp = *reg & 0x0f;
	*reg >>= 4;
	*reg |= tmp << 4;

	cpu->f.z = *reg == 0;
	cpu->f.c = cpu->f.h = cpu->f.n = 0;

	/* [hl] */
	if (opcode == 0x36)
		return 4;

	return 2;
}

static int
srl_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	cpu->f.c = *reg & 1;
	*reg >>= 1;

	cpu->f.z = *reg == 0;

	cpu->f.n = 0;
	cpu->f.h = 0;

	/* [hl] */
	if (opcode == 0x3e)
		return 4;

	return 2;
}

static int
bit_b3_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	uint8_t bit = (opcode >> 3) & 0b111;

	cpu->f.z = (*reg & (1 << bit)) == 0;
	cpu->f.n = 0;
	cpu->f.h = 1;

	if ((opcode & 0b111) == m)
		return 3;

	return 2;
}

static int
res_b3_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	uint8_t bit = (opcode >> 3) & 0b111;

	*reg &= ~(1 << bit);

	if ((opcode & 0b111) == m)
		return 4;

	return 2;
}

static int
set_b3_r8(struct CPU *cpu, uint8_t opcode)
{
	uint8_t *reg = parse_r8(cpu, opcode, 0);

	uint8_t bit = (opcode >> 3) & 0b111;

	*reg |= 1 << bit;

	if ((opcode & 0b111) == m)
		return 4;

	return 2;
}

static int
prefix(struct CPU *cpu)
{
	uint8_t opcode = mem_read(cpu->memory, cpu->pc++);

	if ((opcode & 0b11111000) == 0) {
		return rlc_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b1000) {
		return rrc_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b10000) {
		return rl_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b11000) {
		return rr_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b100000) {
		return sla_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b101000) {
		return sra_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b110000) {
		return swap_r8(cpu, opcode);
	}

	if ((opcode & 0b11111000) == 0b111000) {
		return srl_r8(cpu, opcode);
	}

	if (opcode >> 6 == 1) {
		return bit_b3_r8(cpu, opcode);
	}

	if (opcode >> 6 == 0b10) {
		return res_b3_r8(cpu, opcode);
	}

	if (opcode >> 6 == 0b11) {
		return set_b3_r8(cpu, opcode);
	}

	unimlemented_opcode(opcode);

	return 100;
}

/* TODO: cycles/timing */
static int
handle_interrupt(struct CPU *cpu)
{
	uint8_t flag = mem_read(cpu->memory, IF);
	uint8_t enable = mem_read(cpu->memory, IE);

	if (flag & INTERRUPT_VBLANK && enable & INTERRUPT_VBLANK) {
		fprintf(cpu->log, "int %d ", INTERRUPT_VBLANK);
		mem_write(cpu->memory, IF, flag & ~INTERRUPT_VBLANK);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x40);
		return 5;
	}

	if (flag & INTERRUPT_STAT && enable & INTERRUPT_STAT) {
		fprintf(cpu->log, "int %d ", INTERRUPT_STAT);
		mem_write(cpu->memory, IF, flag & ~INTERRUPT_STAT);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x48);
		return 5;
	}

	if (flag & INTERRUPT_TIMER && enable & INTERRUPT_TIMER) {
		fprintf(cpu->log, "int %d ", INTERRUPT_TIMER);
		mem_write(cpu->memory, IF, flag & ~INTERRUPT_TIMER);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x50);
		return 5;
	}

	if (flag & INTERRUPT_SERIAL && enable & INTERRUPT_SERIAL) {
		fprintf(cpu->log, "int %d ", INTERRUPT_SERIAL);
		mem_write(cpu->memory, IF, flag & ~INTERRUPT_SERIAL);
		cpu->ime = IME_UNSET;

		cpu->pc -= 2;
		call(cpu, 0x00, 0x58);
		return 5;
	}

	if (flag & INTERRUPT_JOYPAD && enable & INTERRUPT_JOYPAD) {
		fprintf(cpu->log, "int %d ", INTERRUPT_JOYPAD);
		mem_write(cpu->memory, IF, flag & ~INTERRUPT_JOYPAD);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x60);
		return 5;
	}

	return 0;
}

int
cpu_execute(struct CPU *cpu)
{
	uint8_t opcode = mem_read(cpu->memory, cpu->pc);

	if (cpu->halt) {
		if (mem_read(cpu->memory, IE) & mem_read(cpu->memory, IF)) {
			cpu->halt = 0;
			if (cpu->ime == 0)
				goto halt_bug; /* TODO: actually test halt bug */
		}
	}

	if (!cpu->halt && !cpu->stop)
		cpu->pc++;
halt_bug:

	switch (opcode) {
		case 0x00: /* NOP */
			return 1;
		break;
		case 0x01:
			return ld_r16_imm16(cpu, opcode);
		break;
		case 0x02:
			return ld_r16mem_a(cpu, opcode);
		break;
		case 0x03:
			return inc_r16(cpu, opcode);
		break;
		case 0x04:
			return inc_r8(cpu, opcode);
		break;
		case 0x05:
			return dec_r8(cpu, opcode);
		break;
		case 0x06:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x07:
			return bit_shift(cpu, opcode);
		break;
		case 0x08:
			return ld_imm16_sp(cpu);
		break;
		case 0x09:
			return add_hl_r16(cpu, opcode);
		break;
		case 0x0a:
			return ld_a_r16mem(cpu, opcode);
		break;
		case 0x0b:
			return dec_r16(cpu, opcode);
		break;
		case 0x0c:
			return inc_r8(cpu, opcode);
		break;
		case 0x0d:
			return dec_r8(cpu, opcode);
		break;
		case 0x0e:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x0f:
			return bit_shift(cpu, opcode);
		break;
		case 0x10:
			return stop(cpu);
		break;
		case 0x11:
			return ld_r16_imm16(cpu, opcode);
		break;
		case 0x12:
			return ld_r16mem_a(cpu, opcode);
		break;
		case 0x13:
			return inc_r16(cpu, opcode);
		break;
		case 0x14:
			return inc_r8(cpu, opcode);
		break;
		case 0x15:
			return dec_r8(cpu, opcode);
		break;
		case 0x16:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x17:
			return bit_shift(cpu, opcode);
		break;
		case 0x18:
			return jr_imm8(cpu, opcode);
		break;
		case 0x19:
			return add_hl_r16(cpu, opcode);
		break;
		case 0x1a:
			return ld_a_r16mem(cpu, opcode);
		break;
		case 0x1b:
			return dec_r16(cpu, opcode);
		break;
		case 0x1c:
			return inc_r8(cpu, opcode);
		break;
		case 0x1d:
			return dec_r8(cpu, opcode);
		break;
		case 0x1e:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x1f:
			return bit_shift(cpu, opcode);
		break;
		case 0x20:
			return jr_imm8(cpu, opcode);
		break;
		case 0x21:
			return ld_r16_imm16(cpu, opcode);
		break;
		case 0x22:
			return ld_r16mem_a(cpu, opcode);
		break;
		case 0x23:
			return inc_r16(cpu, opcode);
		break;
		case 0x24:
			return inc_r8(cpu, opcode);
		break;
		case 0x25:
			return dec_r8(cpu, opcode);
		break;
		case 0x26:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x27:
			return bit_shift(cpu, opcode);
		break;
		case 0x28:
			return jr_imm8(cpu, opcode);
		break;
		case 0x29:
			return add_hl_r16(cpu, opcode);
		break;
		case 0x2a:
			return ld_a_r16mem(cpu, opcode);
		break;
		case 0x2b:
			return dec_r16(cpu, opcode);
		break;
		case 0x2c:
			return inc_r8(cpu, opcode);
		break;
		case 0x2d:
			return dec_r8(cpu, opcode);
		break;
		case 0x2e:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x2f:
			return bit_shift(cpu, opcode);
		break;
		case 0x30:
			return jr_imm8(cpu, opcode);
		break;
		case 0x31:
			return ld_r16_imm16(cpu, opcode);
		break;
		case 0x32:
			return ld_r16mem_a(cpu, opcode);
		break;
		case 0x33:
			return inc_r16(cpu, opcode);
		break;
		case 0x34:
			return inc_r8(cpu, opcode);
		break;
		case 0x35:
			return dec_r8(cpu, opcode);
		break;
		case 0x36:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x37:
			return bit_shift(cpu, opcode);
		break;
		case 0x38:
			return jr_imm8(cpu, opcode);
		break;
		case 0x39:
			return add_hl_r16(cpu, opcode);
		break;
		case 0x3a:
			return ld_a_r16mem(cpu, opcode);
		break;
		case 0x3b:
			return dec_r16(cpu, opcode);
		break;
		case 0x3c:
			return inc_r8(cpu, opcode);
		break;
		case 0x3d:
			return dec_r8(cpu, opcode);
		break;
		case 0x3e:
			return ld_r8_imm8(cpu, opcode);
		break;
		case 0x3f:
			return bit_shift(cpu, opcode);
		break;
		case 0x40:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x41:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x42:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x43:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x44:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x45:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x46:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x47:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x48:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x49:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x4a:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x4b:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x4c:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x4d:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x4e:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x4f:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x50:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x51:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x52:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x53:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x54:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x55:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x56:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x57:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x58:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x59:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x5a:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x5b:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x5c:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x5d:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x5e:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x5f:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x60:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x61:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x62:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x63:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x64:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x65:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x66:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x67:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x68:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x69:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x6a:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x6b:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x6c:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x6d:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x6e:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x6f:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x70:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x71:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x72:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x73:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x74:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x75:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x76:
			return halt(cpu);
		break;
		case 0x77:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x78:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x79:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x7a:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x7b:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x7c:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x7d:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x7e:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x7f:
			return ld_r8_r8(cpu, opcode);
		break;
		case 0x80:
			return add_r8(cpu, opcode);
		break;
		case 0x81:
			return add_r8(cpu, opcode);
		break;
		case 0x82:
			return add_r8(cpu, opcode);
		break;
		case 0x83:
			return add_r8(cpu, opcode);
		break;
		case 0x84:
			return add_r8(cpu, opcode);
		break;
		case 0x85:
			return add_r8(cpu, opcode);
		break;
		case 0x86:
			return add_r8(cpu, opcode);
		break;
		case 0x87:
			return add_r8(cpu, opcode);
		break;
		case 0x88:
			return adc_r8(cpu, opcode);
		break;
		case 0x89:
			return adc_r8(cpu, opcode);
		break;
		case 0x8a:
			return adc_r8(cpu, opcode);
		break;
		case 0x8b:
			return adc_r8(cpu, opcode);
		break;
		case 0x8c:
			return adc_r8(cpu, opcode);
		break;
		case 0x8d:
			return adc_r8(cpu, opcode);
		break;
		case 0x8e:
			return adc_r8(cpu, opcode);
		break;
		case 0x8f:
			return adc_r8(cpu, opcode);
		break;
		case 0x90:
			return sub_r8(cpu, opcode);
		break;
		case 0x91:
			return sub_r8(cpu, opcode);
		break;
		case 0x92:
			return sub_r8(cpu, opcode);
		break;
		case 0x93:
			return sub_r8(cpu, opcode);
		break;
		case 0x94:
			return sub_r8(cpu, opcode);
		break;
		case 0x95:
			return sub_r8(cpu, opcode);
		break;
		case 0x96:
			return sub_r8(cpu, opcode);
		break;
		case 0x97:
			return sub_r8(cpu, opcode);
		break;
		case 0x98:
			return sbc_r8(cpu, opcode);
		break;
		case 0x99:
			return sbc_r8(cpu, opcode);
		break;
		case 0x9a:
			return sbc_r8(cpu, opcode);
		break;
		case 0x9b:
			return sbc_r8(cpu, opcode);
		break;
		case 0x9c:
			return sbc_r8(cpu, opcode);
		break;
		case 0x9d:
			return sbc_r8(cpu, opcode);
		break;
		case 0x9e:
			return sbc_r8(cpu, opcode);
		break;
		case 0x9f:
			return sbc_r8(cpu, opcode);
		break;
		case 0xa0:
			return and_r8(cpu, opcode);
		break;
		case 0xa1:
			return and_r8(cpu, opcode);
		break;
		case 0xa2:
			return and_r8(cpu, opcode);
		break;
		case 0xa3:
			return and_r8(cpu, opcode);
		break;
		case 0xa4:
			return and_r8(cpu, opcode);
		break;
		case 0xa5:
			return and_r8(cpu, opcode);
		break;
		case 0xa6:
			return and_r8(cpu, opcode);
		break;
		case 0xa7:
			return and_r8(cpu, opcode);
		break;
		case 0xa8:
			return xor_r8(cpu, opcode);
		break;
		case 0xa9:
			return xor_r8(cpu, opcode);
		break;
		case 0xaa:
			return xor_r8(cpu, opcode);
		break;
		case 0xab:
			return xor_r8(cpu, opcode);
		break;
		case 0xac:
			return xor_r8(cpu, opcode);
		break;
		case 0xad:
			return xor_r8(cpu, opcode);
		break;
		case 0xae:
			return xor_r8(cpu, opcode);
		break;
		case 0xaf:
			return xor_r8(cpu, opcode);
		break;
		case 0xb0:
			return or_r8(cpu, opcode);
		break;
		case 0xb1:
			return or_r8(cpu, opcode);
		break;
		case 0xb2:
			return or_r8(cpu, opcode);
		break;
		case 0xb3:
			return or_r8(cpu, opcode);
		break;
		case 0xb4:
			return or_r8(cpu, opcode);
		break;
		case 0xb5:
			return or_r8(cpu, opcode);
		break;
		case 0xb6:
			return or_r8(cpu, opcode);
		break;
		case 0xb7:
			return or_r8(cpu, opcode);
		break;
		case 0xb8:
			return cp_r8(cpu, opcode);
		break;
		case 0xb9:
			return cp_r8(cpu, opcode);
		break;
		case 0xba:
			return cp_r8(cpu, opcode);
		break;
		case 0xbb:
			return cp_r8(cpu, opcode);
		break;
		case 0xbc:
			return cp_r8(cpu, opcode);
		break;
		case 0xbd:
			return cp_r8(cpu, opcode);
		break;
		case 0xbe:
			return cp_r8(cpu, opcode);
		break;
		case 0xbf:
			return cp_r8(cpu, opcode);
		break;
		case 0xc0:
			return ret_cond(cpu, opcode);
		break;
		case 0xc1:
			return pop_r16stk(cpu, opcode);
		break;
		case 0xc2:
			return jp_cond(cpu, opcode);
		break;
		case 0xc3:
			return jp_a16(cpu);
		break;
		case 0xc4:
			return call_cond(cpu, opcode);
		break;
		case 0xc5:
			return push_r16stk(cpu, opcode);
		break;
		case 0xc6:
			return add_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xc7:
			return rst(cpu, opcode);
		break;
		case 0xc8:
			return ret_cond(cpu, opcode);
		break;
		case 0xc9:
			return ret(cpu);
		break;
		case 0xca:
			return jp_cond(cpu, opcode);
		break;
		case 0xcb:
			return prefix(cpu);
		break;
		case 0xcc:
			return call_cond(cpu, opcode);
		break;
		case 0xcd:
			return call_a16(cpu);
		break;
		case 0xce:
			return adc_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xcf:
			return rst(cpu, opcode);
		break;
		case 0xd0:
			return ret_cond(cpu, opcode);
		break;
		case 0xd1:
			return pop_r16stk(cpu, opcode);
		break;
		case 0xd2:
			return jp_cond(cpu, opcode);
		break;
		case 0xd3:
			fprintf(stderr, "0xd3 illigal opcode\n");
			exit(1);
		break;
		case 0xd4:
			return call_cond(cpu, opcode);
		break;
		case 0xd5:
			return push_r16stk(cpu, opcode);
		break;
		case 0xd6:
			return sub_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xd7:
			return rst(cpu, opcode);
		break;
		case 0xd8:
			return ret_cond(cpu, opcode);
		break;
		case 0xd9:
			return reti(cpu);
		break;
		case 0xda:
			return jp_cond(cpu, opcode);
		break;
		case 0xdb:
			fprintf(stderr, "0xdb illigal opcode\n");
			exit(1);
		break;
		case 0xdc:
			return call_cond(cpu, opcode);
		break;
		case 0xdd:
			fprintf(stderr, "0xdd illigal opcode\n");
			exit(1);
		break;
		case 0xde:
			return sbc_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xdf:
			return rst(cpu, opcode);
		break;
		case 0xe0:
			return ldh(cpu, opcode);
		break;
		case 0xe1:
			return pop_r16stk(cpu, opcode);
		break;
		case 0xe2:
			return ldh(cpu, opcode);
		break;
		case 0xe3:
			fprintf(stderr, "0xe3 illigal opcode\n");
			exit(1);
		break;
		case 0xe4:
			fprintf(stderr, "0xe4 illigal opcode\n");
			exit(1);
		break;
		case 0xe5:
			return push_r16stk(cpu, opcode);
		break;
		case 0xe6:
			return and_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xe7:
			return rst(cpu, opcode);
		break;
		case 0xe8:
			return add_sp_imm8(cpu);
		break;
		case 0xe9:
			return jp_hl(cpu);
		break;
		case 0xea:
			return ldh(cpu, opcode);
		break;
		case 0xeb:
			fprintf(stderr, "0xeb illigal opcode\n");
			exit(1);
		break;
		case 0xec:
			fprintf(stderr, "0xec illigal opcode\n");
			exit(1);
		break;
		case 0xed:
			fprintf(stderr, "0xed illigal opcode\n");
			exit(1);
		break;
		case 0xee:
			return xor_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xef:
			return rst(cpu, opcode);
		break;
		case 0xf0:
			return ldh(cpu, opcode);
		break;
		case 0xf1:
			return pop_r16stk(cpu, opcode);
		break;
		case 0xf2:
			return ldh(cpu, opcode);
		break;
		case 0xf3:
			return di(cpu);
		break;
		case 0xf4:
			fprintf(stderr, "0xf4 illigal opcode\n");
			exit(1);
		break;
		case 0xf5:
			return push_r16stk(cpu, opcode);
		break;
		case 0xf6:
			return or_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xf7:
			return rst(cpu, opcode);
		break;
		case 0xf8:
			return ldh(cpu, opcode);
		break;
		case 0xf9:
			return ld_sp_hl(cpu);
		break;
		case 0xfa:
			return ldh(cpu, opcode);
		break;
		case 0xfb:
			return ei(cpu);
		break;
		case 0xfc:
			fprintf(stderr, "0xfc illigal opcode\n");
			exit(1);
		break;
		case 0xfd:
			fprintf(stderr, "0xfd illigal opcode\n");
			exit(1);
		break;
		case 0xfe:
			return cp_imm8(cpu, mem_read(cpu->memory, cpu->pc));
		break;
		case 0xff:
			return rst(cpu, opcode);
		break;
		default:
			unimlemented_opcode(opcode);
		break;
	}
	return 0;
}

int
execute(struct CPU *cpu)
{
	if (cpu->ime == IME_NEXT)
		cpu->ime = IME_SET;

	uint8_t cycles = cpu_execute(cpu);
	cpu->mcycles += cycles;
	timer_incr(cpu, cycles);

	if (cpu->ime == IME_SET) {
		if (handle_interrupt(cpu)) {
			cpu->mcycles += 5;
		}
	}
	return cycles;
}

void
cpu_log(struct CPU *cpu)
{
	fprintf(cpu->log, "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X "
		 "L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n",
		cpu->a, cpu->f.flags, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l,
		cpu->sp, cpu->pc, cpu->memory[cpu->pc], cpu->memory[cpu->pc+1],
		cpu->memory[cpu->pc+2], cpu->memory[cpu->pc+3]);
}

void
print_cpu_state(struct CPU *cpu)
{
	fprintf(cpu->log, "%13s|", get_mnemonic(&cpu->memory[cpu->pc]));
	fprintf(cpu->log, "DIV: %08d ", mem_read(cpu->memory, DIV));
	fprintf(cpu->log, "CYC: %05d ", cpu->mcycles);
	fprintf(cpu->log, "TAC: %03b TIMA: %02x ", mem_read(cpu->memory, TAC), mem_read(cpu->memory, TIMA));

	fprintf(cpu->log, "%c%c%c%c ",
		 cpu->f.z ? 'z' : '-', cpu->f.n ? 'n' : '-', cpu->f.h ? 'h' : '-',
		 cpu->f.c ? 'c' : '-');
	fprintf(cpu->log, "IME: %d IF: %05b IE: %05b ", cpu->ime, mem_read(cpu->memory, IF),
		 mem_read(cpu->memory, IE));

	fprintf(cpu->log, "AF: %02x%02x BC: %02x%02x DE: %02x%02x HL: %02x%02x "
		 "SP: %04x PC: %04x ", cpu->a, cpu->f.flags, cpu->b, cpu->c, cpu->d,
		 cpu->e, cpu->h, cpu->l, cpu->sp, cpu->pc);

	fprintf(cpu->log, "[HL]: %02x ", mem_read(cpu->memory, cpu->h << 8 | cpu->l));
	fprintf(cpu->log, "[%02x %02x %02x %02x] ", mem_read(cpu->memory, cpu->sp),
		 mem_read(cpu->memory, cpu->sp+1), mem_read(cpu->memory, cpu->sp+2), mem_read(cpu->memory, cpu->sp+3));

	fprintf(cpu->log, "(%02x %02x %02x %02x)", mem_read(cpu->memory, cpu->pc),
		 mem_read(cpu->memory, cpu->pc+1), mem_read(cpu->memory, cpu->pc+2), mem_read(cpu->memory, cpu->pc+3));

	fprintf(cpu->log, "\n");
}
