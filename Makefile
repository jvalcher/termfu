
# make				- 
# make colors		- check current terminals colors

CC=			gcc
PROD_FLAGS=	-O3
DEV_FLAGS=	-g -Wall

B_FILE=		gdb_tuiffic
CFILES=		$(wildcard ./src/*.c)
OFILES=		$(patsubst ./src/%.c, ./obj/%.o, $(CFILES))

.PHONY: all run

all: FLAGS += $(PROD_FLAGS)
all: $(B_FILE)

dev: FLAGS += $(DEV_FLAGS)
dev: $(B_FILE)
	./$(B_FILE)

$(B_FILE): $(OFILES)
	$(CC) -o $@ $^ -lncurses

obj/%.o: src/%.c
	$(CC) $(FLAGS) -c -o $@ $<

colors: tests/colors_test.c
	./tests/run tests/colors_test.c
