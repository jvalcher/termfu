
# make				- build production binary
# make dev			- build development binary, run
# make colors		- check if current terminal can display colors

CC=			gcc
PROD_FLAGS=	-O3
DEV_FLAGS=	-g -Wall

B_FILE=		gdb-tuiffic
CFILES=		$(wildcard ./src/*.c)
OFILES=		$(patsubst ./src/%.c, ./obj/%.o, $(CFILES))

.PHONY: all dev colors

all: FLAGS += $(PROD_FLAGS)
all: $(B_FILE) prod_rebuild

dev: FLAGS += $(DEV_FLAGS)
dev: $(B_FILE)
	./$(B_FILE)

$(B_FILE): $(OFILES)
	$(CC) -o $@ $^ -lncurses

obj/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

prod_rebuild:

colors: tests/colors_test.c
	@./tests/run tests/colors_test.c
	@rm ./test.out
