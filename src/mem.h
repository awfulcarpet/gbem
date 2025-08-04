#include <stdint.h>

void mem_init(struct CPU *cpu);
int load_rom(uint8_t *mem, char *path);
uint8_t mem_read(uint8_t *mem, uint16_t adr);
void mem_write(uint8_t *mem, uint16_t adr, uint8_t data);
