#include <stdint.h>
struct CPU {
	uint8_t memory[0xFFFF + 1];
};

struct CPU *init_cpu(void);
