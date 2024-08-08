
#
# Commands:
# -------
# 	make      	- build prod binary
# 	make dev	- build dev binary, run it
# 	make devf   - build dev binary with formatted error messages
# 	make debug  - build dev binary with DEBUG flag (no ncurses), run it
# 	make colors	- check if current terminal can display colors
#

B_FILE_DEV=		term_debug_dev
B_FILE_PROD=	term_debug

CC=				gcc
FLAGS=			-Wall -Wextra -MMD -I ./src/update_window_data
PROD_FLAGS=		-O3
DEV_FLAGS=		-g
DEBUG_FLAGS=	-D DEBUG -g
DEV_F_FLAGS=	-g -fdiagnostics-format=json

NCURSES_CFLAGS 	:= $(shell ncurses5-config --cflags)
NCURSES_LIBS 	:= $(shell ncurses5-config --libs)

C_FILES=		$(wildcard ./src/*.c)
O_FILES=		$(patsubst ./src/%.c, ./obj/%.o, $(C_FILES))
CU_FILES=		$(wildcard ./src/update_window_data/*.c)
OU_FILES=		$(patsubst ./src/update_window_data/%.c, ./obj/%.o, $(CU_FILES))
D_FILES= 		$(patsubst ./src/%.c, ./obj/%.d, $(C_FILES)) \
				$(patsubst ./src/update_window_data/%.c, ./obj/%.d, $(CU_FILES))

.PHONY: clean colors all dev debug


# final
#
all: FLAGS += $(PROD_FLAGS)
all: clean $(B_FILE_PROD)
	@echo ""

dev: FLAGS += $(DEV_FLAGS)
dev: clean $(B_FILE_DEV)
	@echo ""
	./$(B_FILE_DEV) ./misc/hello
	@echo ""

debug: FLAGS += $(DEBUG_FLAGS)
debug: clean $(B_FILE_DEV)
	@echo ""
	./$(B_FILE_DEV) ./misc/hello
	@echo ""

devf: FLAGS += $(DEV_F_FLAGS)
devf: clean $(B_FILE_DEV)
	@echo ""

colors:
	./scripts/run ./scripts/colors_test.c
	@rm ./scripts/colors_test


# binary
#
$(B_FILE_PROD): $(O_FILES) $(OU_FILES)
	@echo ""
	$(CC) -o $@ $^ $(NCURSES_LIBS)

$(B_FILE_DEV): $(O_FILES) $(OU_FILES)
	@echo ""
	$(CC) -o $@ $^ $(NCURSES_LIBS)


# objects
#
obj/%.o: src/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<

obj/%.o: src/update_window_data/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<


# d files
#
-include $(D_FILES)



# clean
#
clean:
	rm -f ./obj/*
	rm -f ./term_debug
	rm -f ./term_debug_dev
	@echo ""

