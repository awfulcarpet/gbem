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
	int op = (opcode[0] & 0b00110000) >> 4;
	uint16_t reg = 0;
	uint8_t *high, *low;
	high = low = NULL;

	switch (op) {
		case bc:
			get_r16(cpu->b, cpu->c);
		break;
		case de:
			get_r16(cpu->d, cpu->e);
		break;
		case hl:
			get_r16(cpu->h, cpu->l);
		break;
		case sp:
			reg = cpu->sp;
		break;
		default:
			fprintf(stderr, "incorrect inc instr\n");
			exit(1);
		break;
	};

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
	int op = (opcode[0] & 0b00110000) >> 4;
	uint16_t reg = 0;
	uint8_t *high, *low;
	high = low = NULL;

	switch (op) {
		case bc:
			get_r16(cpu->b, cpu->c);
		break;
		case de:
			get_r16(cpu->d, cpu->e);
		break;
		case hl:
			get_r16(cpu->h, cpu->l);
		break;
		case sp:
			reg = cpu->sp;
		break;
		default:
			fprintf(stderr, "incorrect dec instr\n");
			exit(1);
		break;
	};

	reg--;

	if (op == sp) {
		cpu->sp = reg;
	} else {
		*high = reg >> 8;
		*low = reg & 0xff;
	}

	return 2;
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
		else if ((*opcode & 0b1111) == 0b0011) {
			return inc_r16(cpu, opcode);
		}

		/* dec r16 */
		else if ((*opcode & 0b1111) == 0b1011) {
			return dec_r16(cpu, opcode);
		} else {
			unimlemented_opcode(*opcode);
		}
	}

	unimlemented_opcode(*opcode);

	return 1;
}
