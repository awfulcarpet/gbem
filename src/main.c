#include <sys/types.h>

#include "ram.h"
#include "cpu.h"

int
main(int argc, char **argv) {
	// unused for now
	(void)argc;
	(void)argv;

	struct CPU *cpu = init_cpu();

	for (int i = 0; i <= 0xFF; i++) {
		cpu->memory[i] = i;
	}

	for (int i = 0; i <= 0xFF; i++) {
		cpu->mcycles += execute(cpu);
		print_cpu_state(cpu);
	}
	return 0;
}
