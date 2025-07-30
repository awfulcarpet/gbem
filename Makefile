WARNINGS = -Wall -Wextra -Wpedantic -Wno-unused-result -Wwrite-strings -Wcast-align -Wpointer-arith -Wunused-parameter -Wmissing-include-dirs
CFLAGS = -std=c99 -O0 $(WARNINGS) $(DEFS)
LDLIBS =
TESTS ?= TESTALL

NAME = gbem
OUTDIR = .build
OBJ = \
	  $(OUTDIR)/main.o \
	  $(OUTDIR)/cpu.o \
	  $(OUTDIR)/opcode.o \
	  $(OUTDIR)/ram.o \

TEST_OBJ = \
	  $(OUTDIR)/cpu.o \
	  $(OUTDIR)/opcode.o \
	  $(OUTDIR)/ram.o \

all: $(NAME)

run: $(NAME)
	$(OUTDIR)/$(NAME)

tests: make_tests
	$(OUTDIR)/instr > /dev/null

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(NAME): $(OBJ)
	$(CC) -o $(OUTDIR)/$@ $^ $(LDLIBS)

make_tests: $(TEST_OBJ) tests/instr.c tests/cJSON.o
	$(CC) -o $(OUTDIR)/instr $^ $(LDLIBS) -D$(TESTS)

clean:
	rm -rf $(OUTDIR) core
