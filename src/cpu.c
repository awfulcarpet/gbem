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

uint8_t m_cycles[] = {
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, /* 0x00..0x0f */
};

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

int
execute(struct CPU *cpu) {
	uint8_t *opcode = &cpu->memory[cpu->pc];
	cpu->pc++;

	print_mnemonic(opcode);
	printf("\n");

	return m_cycles[*opcode];
}
