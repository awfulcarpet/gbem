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

	printf("\n");
}

static int
inc_r16(struct CPU *cpu, uint8_t *opcode)
{
	set_regs_r16(0b00110000, 4)

	reg++;

	if (op == sp) {
		cpu->sp = reg;
	} else {
		*high = reg >> 8;
		*low = reg & 0xff;
	}

	return 2;
}

static int
dec_r16(struct CPU *cpu, uint8_t *opcode)
{
	set_regs_r16(0b00110000, 4)

	reg--;

	if (op == sp) {
		cpu->sp = reg;
	} else {
		*high = reg >> 8;
		*low = reg & 0xff;
	}

	return 2;
}

static void
set_zn(struct CPU *cpu, uint8_t reg, uint8_t n)
{
	cpu->f.n = n;
	cpu->f.z = (reg == 0);
}

static int
dec_r8(struct CPU *cpu, uint8_t *opcode)
{
	set_regs_r8(0b00111000, 3)

	(*reg)--;

	set_zn(cpu, *reg, 1);
	cpu->f.h = (*reg & 0b1111) == 0b1111;

	if (op == m)
		return 3;
	return 1;
}

static int
inc_r8(struct CPU *cpu, uint8_t *opcode)
{
	set_regs_r8(0b00111000, 3)

	(*reg)++;

	set_zn(cpu, *reg, 0);
	cpu->f.h = (*reg & 0b1111) == 0b0000;

	if (op == m)
		return 3;
	return 1;
}

static int
ld_r16_imm16(struct CPU *cpu, uint8_t *opcode)
{

	set_regs_r16(0b00110000, 4)
	reg = cpu->memory[cpu->pc + 1] << 8 | cpu->memory[cpu->pc];

	if (op == sp) {
		cpu->sp = reg;
	} else {
		*high = reg >> 8;
		*low = reg & 0xff;
	}
	cpu->pc += 2;

	return 3;
}

int
execute(struct CPU *cpu) {
	uint8_t *opcode = &cpu->memory[cpu->pc];
	cpu->pc++;

	/* block 0 opcodes */
	if (*opcode <= 0x3f && *opcode >= 0x00) {

		/* NOP */
		if (*opcode == 0x00)
			return 1;

		/* inc r16 */
		if ((*opcode & 0b1111) == 0b0011) {
			return inc_r16(cpu, opcode);
		}

		/* dec r16 */
		if ((*opcode & 0b1111) == 0b1011) {
			return dec_r16(cpu, opcode);
		}

		/* inc r8 */
		if ((*opcode & 0b111) == 0b100) {
			return inc_r8(cpu, opcode);
		}

		/* dec r8 */
		if ((*opcode & 0b111) == 0b101) {
			return dec_r8(cpu, opcode);
		}

		if ((*opcode & 0b0001) == 0b0001) {
			return ld_r16_imm16(cpu, opcode);
		}

		unimlemented_opcode(*opcode);
	}

	unimlemented_opcode(*opcode);

	return 1;
}
