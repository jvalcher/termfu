
#
#   -------
#   make      		   - Build production binary
#   make dev		   - Build development binary, run it
#   make devf   	   - Build development binary, print formatted error messages
#   make debug         - Run Tmux debugging session (see scripts/gdb_debug)
#   make todo          - Print source code tags in source code (TODO, FIXME, etc.)
#   make colors		   - Check if current terminal can display colors
#   -------
#


B_FILE_PROD    = termfu
B_FILE_DEV     = termfu_dev
B_FILE_TEST    = termfu_test

CC             = gcc
FLAGS          = -Wall -Wextra
PROD_FLAGS     = -O3
DEV_FLAGS      = -g
FORMAT_FLAGS   = -g -fdiagnostics-format=json

NCURSES_CFLAGS = $(shell ncurses5-config --cflags)
NCURSES_LIBS   = $(shell ncurses5-config --libs)

C_FILES        = $(wildcard ./src/*.c)
C_TEST_FILES   = $(filter-out ./src/main.c, $(wildcard ./src/*.c))
C_UPDATE_FILES = $(wildcard ./src/update_window_data/*.c)
C_POPUP_FILES  = $(wildcard ./src/get_popup_window_input/*.c)


.PHONY: all allf dev devf todo colors clean_prod clean_dev


all: FLAGS   += $(PROD_FLAGS)
all: C_FILES += $(C_UPDATE_FILES)
all: C_FILES += $(C_POPUP_FILES)
all: clean_prod $(B_FILE_PROD)
	@echo ""


dev: FLAGS   += $(DEV_FLAGS)
dev: C_FILES += $(C_UPDATE_FILES)
dev: C_FILES += $(C_POPUP_FILES)
dev: clean_dev $(B_FILE_DEV)
	./$(B_FILE_DEV)
	@echo ""


devf:
	@echo ""
	@./scripts/make_format
	@echo ""

devformat: FLAGS   += $(FORMAT_FLAGS)
devformat: C_FILES += $(C_UPDATE_FILES)
devformat: C_FILES += $(C_POPUP_FILES)
devformat: clean_dev $(B_FILE_DEV)


debug:
	@echo ""
	./scripts/gdb_debug
	@echo ""


todo:
	@echo ""
	./scripts/todo
	@echo ""


colors:
	@echo ""
	./scripts/run ./tests/colors_test.c
	@echo ""
	rm ./tests/colors_test
	@echo ""


clean_prod:
	@echo ""
	rm -f $(B_FILE_PROD)
	@echo ""

clean_dev:
	@echo ""
	rm -f $(B_FILE_DEV)
	@echo ""


$(B_FILE_PROD):
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) $(C_FILES) -o $(B_FILE_PROD) $(NCURSES_LIBS)
	@echo ""

$(B_FILE_DEV):
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) $(C_FILES) -o $(B_FILE_DEV) $(NCURSES_LIBS)
	@echo ""

$(B_FILE_TEST):
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) $(C_TEST_FILES) -o $(B_FILE_TEST) $(NCURSES_LIBS)
	@echo ""

