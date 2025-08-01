#include <stdint.h>

uint8_t read(struct CPU *cpu, uint16_t adr);
void write(struct CPU *cpu, uint16_t adr, uint8_t data);
