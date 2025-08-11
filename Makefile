WARNINGS = -ggdb -Wall -Wextra -Wpedantic -Wno-unused-result -Wwrite-strings -Wcast-align -Wpointer-arith -Wunused-parameter -Wmissing-include-dirs
CFLAGS = -std=c23 $(WARNINGS) $(DEFS)
LDLIBS = -lSDL2
# TESTS ?= -D TEST

NAME = gbem
OUTDIR = .build
OBJ = \
	  $(OUTDIR)/cpu.o \
	  $(OUTDIR)/opcode.o \
	  $(OUTDIR)/mem.o \
	  $(OUTDIR)/timer.o \
	  $(OUTDIR)/ppu.o \
	  $(OUTDIR)/gb.o \
	  $(OUTDIR)/joypad.o \

all: $(NAME)

run: $(NAME)
	$(OUTDIR)/$(NAME) tetris.gb

blargg: $(OBJ) tests/blargg.c
	$(CC) -o $(OUTDIR)/blargg $^ $(LDLIBS)

acid: $(OBJ) tests/acid.c
	$(CC) -o $(OUTDIR)/acid $^ $(LDLIBS)
	$(OUTDIR)/acid

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -c $(CFLAGS) -o $@ $< $(TESTS)

$(NAME): $(OBJ) $(OUTDIR)/main.o
	$(CC) -o $(OUTDIR)/$@ $^ $(LDLIBS)

sm83: $(OBJ) tests/instr.c tests/cJSON.o
	$(CC) -o $(OUTDIR)/instr $^ $(LDLIBS) $(TESTS)
	$(OUTDIR)/instr

clean:
	rm -rf $(OUTDIR) core
