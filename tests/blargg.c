#include "../src/cpu.h"
#include "../src/mem.h"
#include "../src/opcode.h"
#include "../src/gb.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int
main(int argc, char **argv)
{
	uint8_t mem[1 << 16] = {0};
	struct GB *gb = gb_init();

	 char *roms[] = {
		"tests/blargg/cpu_instrs/individual/01-special.gb",
		"tests/blargg/cpu_instrs/individual/02-interrupts.gb",
		"tests/blargg/cpu_instrs/individual/03-op sp,hl.gb",
		"tests/blargg/cpu_instrs/individual/04-op r,imm.gb",
		"tests/blargg/cpu_instrs/individual/05-op rp.gb",
		"tests/blargg/cpu_instrs/individual/06-ld r,r.gb",
		"tests/blargg/cpu_instrs/individual/07-jr,jp,call,ret,rst.gb",
		"tests/blargg/cpu_instrs/individual/08-misc instrs.gb",
		"tests/blargg/cpu_instrs/individual/09-op r,r.gb",
		"tests/blargg/cpu_instrs/individual/10-bit ops.gb",
		"tests/blargg/cpu_instrs/individual/11-op a,(hl).gb",
		"tests/blargg/halt_bug.gb",
		"tests/blargg/instr_timing/instr_timing.gb",
	};

	if (argc < 2) {
		printf("specify test\n");
		return 1;
	}

	if (load_rom(gb->mem, roms[atoi(argv[1]) - 1])) {
		printf("unable to open rom\n");
		return 1;
	}

	gb_run(gb);

	return 0;
}
