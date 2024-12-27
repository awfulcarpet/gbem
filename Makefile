WARNINGS = -Wall -Wextra -Wpedantic -Wno-unused-result -Wwrite-strings -Wcast-align -Wpointer-arith -Wunused-parameter -Wmissing-include-dirs
CFLAGS = -std=c11 -O0 $(WARNINGS)
LDLIBS =

NAME = gbem
OUTDIR = .build
OBJ = \
	  $(OUTDIR)/main.o \
	  $(OUTDIR)/cpu.o \
	  $(OUTDIR)/opcode.o \
	  $(OUTDIR)/ram.o \

all: $(NAME)

run: $(NAME)
	$(OUTDIR)/$(NAME)

$(OUTDIR)/%.o: src/%.c
	@mkdir -p $(OUTDIR)
	$(CC) -c $(CFLAGS) -o $@ $<

$(NAME): $(OBJ)
	$(CC) -o $(OUTDIR)/$@ $^ $(LDLIBS)

clean:
	rm -rf $(OUTDIR) core
