#include <stdio.h>
#include <sys/types.h>

#include "ram.h"
#include "cpu.h"

int
main(int argc, char **argv) {
	// unused for now
	(void)argc;
	(void)argv;

	struct CPU *cpu = init_cpu();
	write(cpu->memory, 0x8e00, 0xca);
	write(cpu->memory, 0x8e01, 0xfe);

	printf("%02x%02x\n", read(cpu->memory, 0x8e00), read(cpu->memory, 0x8e01));
	return 0;
}
