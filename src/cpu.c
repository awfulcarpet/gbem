#include <stdlib.h>
#include <string.h>

#include "cpu.h"

struct CPU *
init_cpu(void) {
	struct CPU *cpu = calloc(1, sizeof(struct CPU));
	memset(cpu->memory, 0, 0xFFFF + 1);

	return cpu;
}
