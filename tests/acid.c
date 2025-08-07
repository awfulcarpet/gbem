#include "../src/gb.h"
#include "../src/mem.h"

int
main(void)
{
	struct GB *gb = gb_init();
	if(load_rom(gb->mem, "tests/dmg-acid2.gb"))
		return 1;


	gb_run(gb);

	return 0;
}
