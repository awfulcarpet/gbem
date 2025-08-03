#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "ram.h"
#include "timer.h"

static uint8_t sum = 0;

static void
incr(struct CPU *cpu, uint8_t cycles, uint16_t period) {
	uint8_t tima = read(cpu, TIMA);

	sum += cycles;

	while (sum >= period) {
		sum -= period;
		write(cpu, TIMA, ++tima);

		if (tima == 0x00) {
			write(cpu, TIMA, read(cpu, TMA));
			request_interrupt(cpu, INTERRUPT_TIMER);
		}
	}
}

void
timer_incr(struct CPU *cpu, int cycles)
{
	uint8_t tac = read(cpu, TAC);
	cpu->div += cycles * 4;

	write(cpu, DIV, cpu->div >> 8);

	if ((tac & TAC_ENABLE) == 0)
		return;

	switch (tac & TAC_CLOCK) {
		case 0:
			incr(cpu, cycles, 256);
		break;
		case 1:
			incr(cpu, cycles, 4);
		break;
		case 2:
			incr(cpu, cycles, 16);
		break;
		case 3:
			incr(cpu, cycles, 64);
		break;
		default:
			assert(NULL); /* unreachable */
		break;
	}
}
