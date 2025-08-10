WARNINGS = -ggdb -Wall -Wextra -Wpedantic -Wno-unused-result -Wwrite-strings -Wcast-align -Wpointer-arith -Wunused-parameter -Wmissing-include-dirs
CFLAGS = -std=c99 $(WARNINGS) $(DEFS)
LDLIBS = -lSDL2
TESTS ?= TEST

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

tests: make_tests blargg acid
	$(OUTDIR)/instr
	$(OUTDIR)/acid
	# rm -f /tmp/log
	$(OUTDIR)/blargg 1 &

blargg: $(OBJ) tests/blargg.c
	$(CC) -o $(OUTDIR)/blargg $^ -D$(TESTS) $(LDLIBS)

acid: $(OBJ) tests/acid.c
	$(CC) -o $(OUTDIR)/acid $^ -D$(TESTS) $(LDLIBS)
	$(OUTDIR)/acid

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -c $(CFLAGS) -o $@ $< -D$(TESTS)

$(NAME): $(OBJ) $(OUTDIR)/main.o
	$(CC) -o $(OUTDIR)/$@ $^ $(LDLIBS)

make_tests: $(OBJ) tests/instr.c tests/cJSON.o
	$(CC) -o $(OUTDIR)/instr $^ $(LDLIBS) -D$(TESTS)

clean:
	rm -rf $(OUTDIR) core
