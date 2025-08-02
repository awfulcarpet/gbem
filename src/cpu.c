#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "ram.h"
#include "opcode.h"
#include "timer.h"

static FILE *log = NULL;

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

	cpu->ime = 0;

	cpu->mcycles = 0;

	cpu->div_sum = cpu->tima_sum = 0;

	ram_init(cpu);
	memset(cpu->memory, 0, 0xFFFF + 1);

	log = fopen("log", "w");

	return cpu;
}

void
request_interrupt(struct CPU *cpu, enum INTERRUPT interrupt)
{
	write(cpu, IF, interrupt);
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
		*h = read(cpu, cpu->sp - 1);
	if (l != NULL)
		*l = read(cpu, cpu->sp - 2);

	return read(cpu, cpu->sp - 1) << 8 | read(cpu, cpu->sp - 2);
}

static void
push(struct CPU *cpu, uint8_t h, uint8_t l)
{
	write(cpu, --cpu->sp, h);
	write(cpu, --cpu->sp, l);
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

	reg = read(cpu, cpu->pc + 1) << 8 | read(cpu, cpu->pc);

	set_r8_from_r16()
	cpu->pc += 2;

	return 3;
}

static int
ld_r16mem_a(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16mem(0b00110000, 4)

	write(cpu, reg, cpu->a);

	if (op == hli)
		inc_r16(cpu, hl << 4);
	if (op == hld)
		dec_r16(cpu, hl << 4);

	return 2;
}

static int
ld_a_r16mem(struct CPU *cpu, uint8_t opcode)
{
	set_regs_r16mem(0b00110000, 4)

	cpu->a = read(cpu, reg);

	if (op == hli)
		inc_r16(cpu, hl << 4);
	if (op == hld)
		dec_r16(cpu, hl << 4);
	return 2;
}

static int
ld_imm16_sp(struct CPU *cpu, uint8_t opcode)
{
	uint16_t adr = read(cpu, cpu->pc + 1) << 8 | read(cpu, cpu->pc);

	write(cpu, adr, cpu->sp & 0xff);
	write(cpu, adr + 1, cpu->sp >> 8);
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

	*dst = read(cpu, cpu->pc);

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
	cpu->pc += (int8_t)read(cpu, cpu->pc) + 1;
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
	ei(cpu);
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
	jp(cpu, read(cpu, cpu->pc + 1), read(cpu, cpu->pc));
	return 4;
}

static int
jp_hl(struct CPU *cpu)
{
	jp(cpu, cpu->h, cpu->l);
	return 4;
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
	jp(cpu, read(cpu, cpu->pc + 1), read(cpu, cpu->pc));
	return 4;
}

static int
call_a16(struct CPU *cpu)
{
	call(cpu, read(cpu, cpu->pc + 1), read(cpu, cpu->pc));
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
	call(cpu, read(cpu, cpu->pc + 1), read(cpu, cpu->pc));
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
	int op = (opcode & 0b00110000) >> 4;
	uint16_t reg = 0;
	uint8_t *high, *low;
	high = low = NULL;

	switch (op) {
		case s_bc:
			get_r16(cpu->b, cpu->c);
			break;
		case s_de:
			get_r16(cpu->d, cpu->e);
			break;
		case s_hl:
			get_r16(cpu->h, cpu->l);
			break;
		case s_af:
			get_r16(cpu->a, cpu->f.flags);
			break;
	};

	pop(cpu, high, low);

	if (op == s_af)
		*low &= 0xf0;
	return 3;
}

static int
push_r16stk(struct CPU *cpu, uint8_t opcode)
{
	int op = (opcode & 0b00110000) >> 4;
	uint16_t reg = 0;
	uint8_t *high, *low;
	high = low = NULL;

	switch (op) {
		case s_bc:
			get_r16(cpu->b, cpu->c);
			break;
		case s_de:
			get_r16(cpu->d, cpu->e);
			break;
		case s_hl:
			get_r16(cpu->h, cpu->l);
			break;
		case s_af:
			get_r16(cpu->a, cpu->f.flags);
			break;
	};

	write(cpu, --cpu->sp, *high);
	write(cpu, --cpu->sp, *low);

	return 4;
}

static int
ldh(struct CPU *cpu, const uint8_t opcode)
{
	switch (opcode) {
		case 0xE0:
			write(cpu, 0xFF00 + read(cpu, cpu->pc++), cpu->a);
			return 3;
		break;
		case 0xE2:
			write(cpu, 0xFF00 + cpu->c, cpu->a);
			return 2;
		break;
		case 0xEA:
			write(cpu, read(cpu, cpu->pc++) | read(cpu, cpu->pc++) << 8, cpu->a);
			return 4;
		break;
		case 0xf0:
			cpu->a = read(cpu, 0xFF00 + read(cpu, cpu->pc++));
			return 3;
		break;
		case 0xf2:
			cpu->a = read(cpu, 0xFF00 + cpu->c);
			return 2;
		break;
		case 0xf8: {
			uint8_t e = read(cpu, cpu->pc++);

			cpu->f.z = 0;
			cpu->f.n = 0;

			set_hc(cpu, e, cpu->sp);

			cpu->h = ((cpu->sp + (int8_t)e) & 0xFF00) >> 8;
			cpu->l = (cpu->sp + (int8_t)e) & 0xFF;
			return 3;
			break;
		}
		case 0xfa:
			 cpu->a = read(cpu, read(cpu, cpu->pc++) | read(cpu, cpu->pc++) << 8);
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
	uint8_t e = read(cpu, cpu->pc++);

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
	uint8_t opcode = read(cpu, cpu->pc++);

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
	uint8_t flag = read(cpu, IF);
	uint8_t enable = read(cpu, IE);

	if (flag & INTERRUPT_VBLANK && enable & INTERRUPT_VBLANK) {
		fprintf(log, "int %d ", INTERRUPT_VBLANK);
		write(cpu, IF, flag & ~INTERRUPT_VBLANK);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x40);
		return 5;
	}

	if (flag & INTERRUPT_LCD && enable & INTERRUPT_LCD) {
		fprintf(log, "int %d ", INTERRUPT_LCD);
		write(cpu, IF, flag & ~INTERRUPT_LCD);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x48);
		return 5;
	}

	if (flag & INTERRUPT_TIMER && enable & INTERRUPT_TIMER) {
		fprintf(log, "int %d ", INTERRUPT_TIMER);
		write(cpu, IF, flag & ~INTERRUPT_TIMER);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x50);
		return 5;
	}

	if (flag & INTERRUPT_SERIAL && enable & INTERRUPT_SERIAL) {
		fprintf(log, "int %d ", INTERRUPT_SERIAL);
		write(cpu, IF, flag & ~INTERRUPT_SERIAL);
		cpu->ime = IME_UNSET;

		cpu->pc -= 2;
		call(cpu, 0x00, 0x58);
		return 5;
	}

	if (flag & INTERRUPT_JOYPAD && enable & INTERRUPT_JOYPAD) {
		fprintf(log, "int %d ", INTERRUPT_JOYPAD);
		write(cpu, IF, flag & ~INTERRUPT_JOYPAD);
		cpu->ime = IME_UNSET;
		cpu->pc -= 2;
		call(cpu, 0x00, 0x60);
		return 5;
	}

	return 0;
}

static int
execute_opcode(struct CPU *cpu) {
	uint8_t opcode = read(cpu, cpu->pc);

	if (!cpu->halt && !cpu->stop)
		cpu->pc++;

	/* block 0 opcodes */
	if (opcode <= 0x3f && opcode >= 0x00) {

		/* NOP */
		if (opcode == 0x00)
			return 1;


		/* ld r16,imm16 */
		if ((opcode & 0b1111) == 0b0001) {
			return ld_r16_imm16(cpu, opcode);
		}

		/* ld [r16mem],a */
		if ((opcode & 0b1111) == 0b0010) {
			return ld_r16mem_a(cpu, opcode);
		}

		/* ld a,[r16mem] */
		if ((opcode & 0b1111) == 0b1010) {
			return ld_a_r16mem(cpu, opcode);
		}

		/* ld [imm16], sp */
		if (opcode == 0b00001000) {
			return ld_imm16_sp(cpu, opcode);
		}

		/* inc r16 */
		if ((opcode & 0b1111) == 0b0011) {
			return inc_r16(cpu, opcode);
		}

		/* dec r16 */
		if ((opcode & 0b1111) == 0b1011) {
			return dec_r16(cpu, opcode);
		}

		/* add hl, r16 */
		if ((opcode & 0b1111) == 0b1001) {
			return add_hl_r16(cpu, opcode);
		}

		/* inc r8 */
		if ((opcode & 0b111) == 0b100) {
			return inc_r8(cpu, opcode);
		}

		/* dec r8 */
		if ((opcode & 0b111) == 0b101) {
			return dec_r8(cpu, opcode);
		}

		/* ld r8 imm8 */
		if ((opcode & 0b111) == 0b110) {
			return ld_r8_imm8(cpu, opcode);
		}

		/* bit shifts */
		if ((opcode & 0b111) == 0b111) {
			return bit_shift(cpu, opcode);
		}

		/* jr e8 */
		/* jr cond, e8 */
		if (opcode == 0b00011000 || (opcode & 0b00100111) == 0b00100000) {
			return jr_imm8(cpu, opcode);
		}

		/* stop */
		if (opcode == 0x10)
			return stop(cpu);

		unimlemented_opcode(opcode);
	}

	/* block 1 */
	if (opcode >= 0x40 && opcode <= 0x7F) {

		/* halt */
		/* TODO: implement waking from halt*/
		if (opcode == 0x76) {
			return halt(cpu);
		}

		return ld_r8_r8(cpu, opcode);
	}

	/* block 2 */
	if (opcode >= 0x80 && opcode <= 0xbf) {
		uint8_t op = opcode >> 3 & 0b111;

		if (op == 0b000) {
			return add_r8(cpu, opcode);
		}

		if (op == 0b001) {
			return adc_r8(cpu, opcode);
		}

		if (op == 0b010) {
			return sub_r8(cpu, opcode);
		}

		if (op == 0b011) {
			return sbc_r8(cpu, opcode);
		}

		if (op == 0b100) {
			return and_r8(cpu, opcode);
		}

		if (op == 0b101) {
			return xor_r8(cpu, opcode);
		}

		if (op == 0b110) {
			return or_r8(cpu, opcode);
		}

		if (op == 0b111) {
			return cp_r8(cpu, opcode);
		}

		unimlemented_opcode(opcode);
	}

	/* block 3 */
	/* 8bit arith with imm8 */
	if ((opcode & 0b11000111) == 0b11000110) {
		uint8_t op = opcode >> 3 & 0b111;

		switch (op) {
			case 0:
				return add_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 1:
				return adc_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 2:
				return sub_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 3:
				return sbc_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 4:
				return and_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 5:
				return xor_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 6:
				return or_imm8(cpu, read(cpu, cpu->pc));
				break;
			case 7:
				return cp_imm8(cpu, read(cpu, cpu->pc));
				break;
		}

		unimlemented_opcode(opcode);
	}

	if (opcode == 0xc9)
		return ret(cpu);
	if (opcode == 0xd9)
		return reti(cpu);

	if ((opcode & 0b11100111) == 0b11000000) {
		return ret_cond(cpu, opcode);
	}

	if (opcode == 0xc3)
		return jp_a16(cpu);
	if (opcode == 0xe9)
		return jp_hl(cpu);
	if ((opcode & 0b11100111) == 0b11000010) {
		return jp_cond(cpu, opcode);
	}

	if (opcode == 0xcd)
		return call_a16(cpu);
	if ((opcode & 0b11100111) == 0b11000100) {
		return call_cond(cpu, opcode);
	}

	if ((opcode & 0b11000111) == 0b11000111) {
		return rst(cpu, opcode);
	}

	if ((opcode & 0b11001111) == 0b11000001) {
		return pop_r16stk(cpu, opcode);
	}

	if ((opcode & 0b11001111) == 0b11000101) {
		return push_r16stk(cpu, opcode);
	}

	if (opcode == 0xE8) {
		return add_sp_imm8(cpu);
	}

	if ((opcode & 0b11100101) == 0b11100000
			|| (opcode & 0b111100101) == 0b11110000) {
		return ldh(cpu, opcode);
	}

	if (opcode == 0xf9) {
		return ld_sp_hl(cpu);
	}

	if (opcode == 0xf3)
		return di(cpu);
	if (opcode == 0xfb)
		return ei(cpu);

	if (opcode == 0xcb)
		return prefix(cpu);

	unimlemented_opcode(opcode);

	return 1;
}

void
execute(struct CPU *cpu)
{
	timer_incr(cpu);
	if (cpu->ime == IME_NEXT)
		cpu->ime = IME_SET;

	cpu->mcycles += execute_opcode(cpu);

	if (cpu->ime == IME_SET) {
		if (handle_interrupt(cpu))
			cpu->mcycles += 5;
	}
}

void
cpu_log(struct CPU *cpu)
{
	fprintf(log, "A: %02X F: %02X B: %02X C: %02X D: %02X E: %02X H: %02X "
		 "L: %02X SP: %04X PC: 00:%04X (%02X %02X %02X %02X)\n",
		cpu->a, cpu->f.flags, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l,
		cpu->sp, cpu->pc, cpu->memory[cpu->pc], cpu->memory[cpu->pc+1],
		cpu->memory[cpu->pc+2], cpu->memory[cpu->pc+3]);
}

void
print_cpu_state(struct CPU *cpu)
{
	fprintf(log, "->%s ", get_mnemonic(read(cpu, cpu->pc)));
	fprintf(log, "CYC: %05d ", cpu->mcycles);
	fprintf(log, "TAC: %08b TIMA: %02x ", read(cpu, TAC), read(cpu, TIMA));

	fprintf(log, "%c%c%c%c ",
		cpu->f.z ? 'z' : '-', cpu->f.n ? 'n' : '-', cpu->f.h ? 'h' : '-', cpu->f.c ? 'c' : '-');
	fprintf(log, "IME: %d IF: %08b IE: %08b ", cpu->ime, read(cpu, IF), read(cpu, IE));

	fprintf(log, "AF: %02x%02x BC: %02x%02x DE: %02x%02x HL: %02x%02x SP: %04x PC: %04x ",
		cpu->a, cpu->f.flags, cpu->b, cpu->c, cpu->d, cpu->e, cpu->h, cpu->l, cpu->sp, cpu->pc);

	fprintf(log, "[HL]: %02x Stk: %02x %02x %02x %02x ",
		read(cpu, cpu->h << 8 | cpu->l), read(cpu, cpu->sp), read(cpu, cpu->sp+1), read(cpu, cpu->sp+2), read(cpu, cpu->sp+3));

	fprintf(log, "(%02x %02x %02x %02x)", read(cpu, cpu->pc), read(cpu, cpu->pc+1), read(cpu, cpu->pc+2), read(cpu, cpu->pc+3));

	fprintf(log, "\n");
}
