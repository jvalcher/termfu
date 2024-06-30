
#
# Commands:
# -------
# 	make      	- build prod binary
# 	make dev	- build dev binary, run it
# 	make debug  - build dev binary, print debug data (no Ncurses), run it
# 	make devf   - build dev binary, print formatted error messages
# 	make colors	- check if current terminal can display colors
#

B_FILE_DEV=		termide-dev
B_FILE_PROD=	termide

CC=				gcc
#FLAGS=			-Wall -Wextra -Werror -MMD -I ./src/plugins -I ./src/window_updates
FLAGS=			-Wall -Wextra -MMD -I ./src/plugins -I ./src/window_updates
PROD_FLAGS=		-O3
DEV_FLAGS=		-g
DEBUG_FLAGS=	-D DEBUG -g
FORMAT_FLAGS=	-D DEBUG -g -fdiagnostics-format=json

NCURSES_CFLAGS 	:= $(shell ncurses5-config --cflags)
NCURSES_LIBS 	:= $(shell ncurses5-config --libs)

C_FILES=		$(wildcard ./src/*.c)
O_FILES=		$(patsubst ./src/%.c, ./obj/%.o, $(C_FILES))
D_FILES= 		$(patsubst ./src/%.c, ./obj/%.d, $(C_FILES))

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

devf: FLAGS += $(FORMAT_FLAGS)
devf: clean $(B_FILE_DEV)
	@echo ""
	./$(B_FILE_DEV) ./misc/hello
	@echo ""

colors:
	./scripts/run ./scripts/colors_test.c
	@rm ./scripts/colors_test


# binary
#
$(B_FILE_PROD): $(O_FILES)
	@echo ""
	$(CC) -o $@ $^ $(NCURSES_LIBS)

$(B_FILE_DEV): $(O_FILES)
	@echo ""
	$(CC) -o $@ $^ $(NCURSES_LIBS)


# objects
#
obj/%.o: src/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<

obj/%.o: src/plugins/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<

obj/%.o: src/window_updates/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c -o $@ $<


# d files
#
-include $(D_WUPD_FILES)
-include $(D_PLUG_FILES)
-include $(D_FILES)



# clean
#
clean:
	rm -f ./obj/*
	rm -f ./termide
	rm -f ./termide-dev
	@echo ""

