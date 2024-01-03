
# Commands:
	# make				- build production binary termide
	# make dev			- build development binary termide-dev, run it
	# make layouts		- print layout information before program continues  (src/data.h - PRINT_LAYOUTS)
	# make colors		- check if current terminal can display colors
	# make num_plugins	- print number of plugins for setting NUM_PLUGINS in src/data.h

CC=				gcc
FLAGS=			-Wall -MMD -I ./src/plugins
PROD_FLAGS=		-O3
DEV_FLAGS=		-g
LAY_FLAGS=		-D LAYOUT -g
NCURSES_CFLAGS 	:= $(shell ncurses5-config --cflags)
NCURSES_LIBS 	:= $(shell ncurses5-config --libs)

B_FILE_DEV=		termide-dev
B_FILE_PROD=	termide

C_FILES=	$(wildcard ./src/*.c)
O_FILES=	$(patsubst ./src/%.c, ./obj/%.o, $(C_FILES))
CP_FILES=	$(wildcard ./src/plugins/*.c)
OP_FILES=	$(wildcard ./src/plugins/*.c, ./obj/%.o, $(CP_FILES))
D_FILES= 	$(patsubst ./src/%.c, ./obj/%.d, $(C_FILES))

.PHONY: all dev colors


# build

all: FLAGS += $(PROD_FLAGS)
all: clean $(B_FILE_PROD)

dev: FLAGS += $(DEV_FLAGS)
dev: clean $(B_FILE_DEV)
	./$(B_FILE_DEV)

layouts: FLAGS += $(LAY_FLAGS)
layouts: clean $(B_FILE_DEV)
	./$(B_FILE_DEV)

$(B_FILE_PROD): $(OP_FILES) $(O_FILES)
	$(CC) -o $@ $^ $(NCURSES_LIBS)

$(B_FILE_DEV):  $(OP_FILES) $(O_FILES)
	$(CC) -o $@ $^ $(NCURSES_LIBS)

obj/%.o: src/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<

obj/%.o: src/plugins/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<

-include $(D_FILES)

clean:
	rm -f ./obj/*
	rm -f ./termide
	rm -f ./termide-dev


# misc make commands

colors:
	@./scripts/run ./scripts/colors_test.c
	@rm ./scripts/colors_test

num_plugins:
	@gcc -g src/plugins/plugins.c
	@./a.out
	@rm ./a.out
