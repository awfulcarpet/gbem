#include <assert.h>
#include "ram.h"

uint8_t
read(uint8_t *ram, uint16_t adr) {
	return ram[adr];
}

void
write(uint8_t *ram, uint16_t adr, uint8_t data) {
	assert(adr >= 0x8000); // avoid writing ROM

	ram[adr] = data;
}
