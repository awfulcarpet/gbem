#include <stdbool.h>
#include <sys/types.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>

#include "cpu.h"
#include "mem.h"
#include "opcode.h"
#include "timer.h"
#include "graphics.h"
#include "gb.h"

int
main(int argc, char **argv) {
	// unused for now
	(void)argc;
	(void)argv;
	struct GB * gb = gb_init();

	if (gb == NULL) return 1;

	/* load CAFE into DE and BABE into BC and swap them */
	// cpu->memory[0] = 0x11;
	// cpu->memory[1] = 0xFE;
	// cpu->memory[2] = 0xCA;
	//
	// cpu->memory[3] = 0x01;
	// cpu->memory[4] = 0xBE;
	// cpu->memory[5] = 0xBA;
	//
	// cpu->memory[6] = 0x78;
	// cpu->memory[7] = 0x42;
	// cpu->memory[8] = 0x57;
	// cpu->memory[9] = 0x79;
	// cpu->memory[10] = 0x4b;
	// cpu->memory[11] = 0x5f;
	// cpu->memory[12] = 0xe0;
	// cpu->memory[13] = 0x02;
	// cpu->memory[0xff01] = 'c';
	//
	// cpu->memory[14] = 0xcd; /* CALL a16 */
	// cpu->memory[15] = 0x17;
	// cpu->memory[16] = 0x00;
	//
	// cpu->memory[0x0017] = 0xcd; /* CALL a16 */
	// cpu->memory[0x0018] = 0x20;
	// cpu->memory[0x0019] = 0x00;
	//
	gb->cpu->memory[0x0020] = 0x10; /* STOP */
	// write(cpu, TAC, 0b101);

	while (1) {
		print_cpu_state(gb->cpu);
		execute(gb->cpu);

		if (gb->cpu->stop == 1)
			break;
	}
		print_cpu_state(gb->cpu);

	getchar();
	return 0;
}
