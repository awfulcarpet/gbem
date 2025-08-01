#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "ram.h"

static FILE *f = NULL;

uint8_t
read(struct CPU *cpu, uint16_t adr) {
	return cpu->memory[adr];
}

void
write(struct CPU *cpu, uint16_t adr, uint8_t data) {
#ifdef TEST
	assert(adr >= 0x8000); // avoid writing ROM
#endif

	if (adr == 0xff01) {
		f = fopen("/tmp/log", "a");
		if (f == NULL)
			exit(5);
		fprintf(f, "write: %02x\n", data);
	}

	cpu->memory[adr] = data;
}
