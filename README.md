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
```
you may find a few roms here:
- ![snake](https://hh3.gbdev.io/static/database-gb/entries/snake-gb/snake.gb)
