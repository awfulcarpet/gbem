#include <sys/types.h>

#include "ram.h"
#include "cpu.h"

int
main(int argc, char **argv) {
	// unused for now
	(void)argc;
	(void)argv;

	struct CPU *cpu = init_cpu();

	/* load CAFE into DE and BABE into BC and swap them */
	cpu->memory[0] = 0x11;
	cpu->memory[1] = 0xFE;
	cpu->memory[2] = 0xCA;

	cpu->memory[3] = 0x01;
	cpu->memory[4] = 0xBE;
	cpu->memory[5] = 0xBA;

	cpu->memory[6] = 0x78;
	cpu->memory[7] = 0x42;
	cpu->memory[8] = 0x57;
	cpu->memory[9] = 0x79;
	cpu->memory[10] = 0x4b;
	cpu->memory[11] = 0x5f;

	cpu->memory[12] = 0x10; /* STOP */

	print_cpu_state(cpu);
	while (1) {
		cpu->mcycles += execute(cpu);
		print_cpu_state(cpu);

		if (cpu->stop == 1)
			break;
	}
	return 0;
}
