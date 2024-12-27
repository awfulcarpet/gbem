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

	memset(cpu->memory, 0, 0xFFFF + 1);

	return cpu;
}

// TODO return cycles later
int
execute(struct CPU *cpu) {
	uint8_t *opcode = &cpu->memory[cpu->pc];
	cpu->pc++;

	print_mnemonic(opcode);
	printf("\n");
	return 0;
}
