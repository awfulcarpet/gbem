#include <stdint.h>

void ram_init(struct CPU *cpu);
int load_rom(struct CPU *cpu, char *path);
uint8_t read(struct CPU *cpu, uint16_t adr);
void write(struct CPU *cpu, uint16_t adr, uint8_t data);
