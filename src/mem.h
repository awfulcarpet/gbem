#include <stdint.h>

void mem_init(struct CPU *cpu);
int load_rom(struct CPU *cpu, char *path);
uint8_t mem_read(struct CPU *cpu, uint16_t adr);
void mem_write(struct CPU *cpu, uint16_t adr, uint8_t data);
