#include <assert.h>
#include "cpu.h"
#include "ram.h"

uint8_t
read(struct CPU *cpu, uint16_t adr) {
	return cpu->memory[adr];
}

void
write(struct CPU *cpu, uint16_t adr, uint8_t data) {
	#ifdef TEST
		assert(adr >= 0x8000); // avoid writing ROM
	#endif


	cpu->memory[adr] = data;
}
