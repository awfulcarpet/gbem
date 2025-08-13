# gbem
a somewhat functional gameboy emulator written in c

## Compiling
A unix-like operating system is assumed.

1. Install dependencies
* SDL2
2. Run `make` to make the main binary (it will reside by default in .build/gbem)
3. optionally run `make` with either/or arguments of `sm83`, `acid` and/or `blargg`
    to build and run the test suite

Usage:
```bash
$ gbem <game.gb> # replace game.gb with a user-provided rom
$ gbem rom/snake.gb # run snake demo from (https://donaldhays.com/projects/snake/)
```

## Features
1. full sm83 cpu core emulation passing all sm83 tests and blargg cpu_instrs tests
2. somewhat functional ppu emulation passing all but 1 dmg-acid2 tests
3. somewhat working input
4. at least 1 fully functional (homebrew) game

## TODO
1. emulation of both tetris and drmario can make it to the start screen but gameplay/selection of modes freezes/doesn't work
    - this is most likely a timer/div issue
2. support at least mbc1
3. pixel FIFO for ppu?
