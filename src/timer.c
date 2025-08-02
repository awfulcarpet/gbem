#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "cpu.h"
#include "ram.h"
#include "timer.h"


static void
incr(struct CPU *cpu, uint16_t period) {
	uint8_t tima = read(cpu, TIMA);

	cpu->tima_sum += cpu->mcycles;
	if (cpu->tima_sum < period) return;
	cpu->tima_sum -= period;


	if (tima == 0xff) {
		write(cpu, TIMA, read(cpu, TMA));
		request_interrupt(cpu, INTERRUPT_TIMER);
		return;
	}

	write(cpu, TIMA, tima + 1);
}

void
timer_incr(struct CPU *cpu)
{
	uint8_t tac = read(cpu, TAC);

	if ((tac & TAC_ENABLE) == 0)
		return;

	switch (tac & TAC_CLOCK) {
		case 0:
			incr(cpu, 256);
		break;
		case 1:
			incr(cpu, 4);
		break;
		case 2:
			incr(cpu, 16);
		break;
		case 3:
			incr(cpu, 64);
		break;
		default:
			assert(NULL); /* unreachable */
		break;
	}
}
