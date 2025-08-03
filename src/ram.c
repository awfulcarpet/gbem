#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "ram.h"
#include "timer.h"

static FILE *f = NULL;

void
ram_init(struct CPU *cpu)
{

	memset(cpu->memory, 0, 0xFFFF + 1);
}

int
load_rom(struct CPU *cpu, char *path)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL)
		return 1;
	int pos = 0;
	while (fread(&cpu->memory[pos], 1, 1, file)) {
		pos++;
	}
	fclose(file);

	return 0;
}

uint8_t
read(struct CPU *cpu, uint16_t adr) {
	return cpu->memory[adr];
}

void
write(struct CPU *cpu, uint16_t adr, uint8_t data) {
#ifndef TEST
	assert(adr >= 0x8000); // avoid writing ROM
#endif

	f = fopen("/tmp/log", "a");
	if (f == NULL)
		exit(5);

	if (adr == SC && data == 0x81) {
		fprintf(f, "%c", read(cpu, SB));
	}
	fclose(f);

#ifndef TEST
	if (adr == DIV)
		data = 0x00;
#endif

	cpu->memory[adr] = data;
}
