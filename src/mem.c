#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "timer.h"

static FILE *f = NULL;

void
mem_init(struct CPU *cpu)
{
	memset(cpu->memory, 0, 0xFFFF + 1);
}

int
load_rom(uint8_t *mem, char *path)
{
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		fprintf(stderr, "unable to open rom: %s\n", path);
		return 1;
	}

	int pos = 0;
	while (fread(&mem[pos], 1, 1, file)) {
		pos++;
	}
	fclose(file);

	return 0;
}

uint8_t
mem_read(uint8_t *mem, uint16_t adr) {
	return mem[adr];
}

void
mem_write(uint8_t *mem, uint16_t adr, uint8_t data) {
#ifndef TEST
	assert(adr >= 0x8000); // avoid writing ROM
#endif

	if (adr == SC && data == 0x81) {
		f = fopen("/tmp/log", "a");
		if (f == NULL)
			exit(5);

		fprintf(f, "%c", mem_read(mem, SB));
		fclose(f);
	}

#ifndef TEST
	if (adr == DIV)
		data = 0x00;
#endif

	mem[adr] = data;
}
