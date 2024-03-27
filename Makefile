
# Commands:
	# make      		- build production binary  (termide)
	# make dev			- build development binary (termide-dev), run it
	# make layouts		- print layout information (see LAYOUT in src/render_layout.c)
	# make colors		- check if current terminal can display colors

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
OP_FILES=	$(patsubst ./src/plugins/%.c, ./obj/%.o, $(CP_FILES))
D_FILES= 	$(patsubst ./src/%.c, ./obj/%.d, $(C_FILES))

.PHONY: all dev colors

# all
all: FLAGS += $(PROD_FLAGS)
all: clean $(B_FILE_PROD)
	@echo ""

# dev
dev: FLAGS += $(DEV_FLAGS)
dev: clean $(B_FILE_DEV)
	@echo ""
	./$(B_FILE_DEV) hello
	@echo ""

# layouts
layouts: FLAGS += $(LAY_FLAGS)
layouts: clean $(B_FILE_DEV)
	@echo ""
	./$(B_FILE_DEV) hello

# create binaries
#
$(B_FILE_PROD): $(OP_FILES) $(O_FILES)
	@echo ""
	$(CC) -o $@ $^ $(NCURSES_LIBS)

$(B_FILE_DEV):  $(OP_FILES) $(O_FILES)
	@echo ""
	$(CC) -o $@ $^ $(NCURSES_LIBS)

# create objects
#
obj/%.o: src/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<

obj/%.o: src/plugins/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<


-include $(D_FILES)

clean:
	rm -f ./obj/*
	rm -f ./termide
	rm -f ./termide-dev
	@echo ""


# misc makes

colors:
	@./scripts/run ./scripts/colors_test.c
	@rm ./scripts/colors_test

