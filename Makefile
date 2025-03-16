
#
#  $ make help
#

# make
MAKEFLAGS += --no-print-directory

# binaries
B_FILE_PROD    = termfu
B_FILE_DEV     = termfu_dev

# flags
CC             = gcc
FLAGS          = -Wall -Wextra
PROD_FLAGS     = -O3
DEV_FLAGS      = -g
FORMAT_FLAGS   = $(DEV_FLAGS) -fdiagnostics-format=json
NCURSES_CFLAGS = $(shell ncurses6-config --cflags)
NCURSES_LIBS   = $(shell ncurses6-config --libs) -lform

# source files
C_FILES         = $(wildcard ./src/*.c)
C_TEST_FILES    = $(filter-out ./src/main.c, $(wildcard ./src/*.c))
C_UPDATE_FILES  = $(wildcard ./src/update_window_data/*.c)
C_FORM_IN_FILES = $(wildcard ./src/get_form_input/*.c)
C_DEBUG_FILES   = $(wildcard ./src/debug/*.c)

# test programs directory
TEST_PROGS_DIR = test_programs

# configuration, data files
CONFIG_RUN_DEV  = configs/.termfu_run_dev
DATA_RUN_DEV    = configs/.termfu_run_dev_data
CONFIG_DEBUG    = configs/.termfu_debugger
DATA_DEBUG	    = configs/.termfu_debugger_data
CONFIG_DEBUGGED = configs/.termfu_debugged
DATA_DEBUGGED   = configs/.termfu_debugged_data

.PHONY: help all install dev devf devformat debug debug_gdb build run_dev plugins clean_prod clean_dev


all: FLAGS   += $(PROD_FLAGS)
all: C_FILES += $(C_UPDATE_FILES) $(C_FORM_IN_FILES) $(C_DEBUG_FILES)
all: clean_prod $(B_FILE_PROD)
	@echo ""

dev: FLAGS   += $(DEV_FLAGS)
dev: C_FILES += $(C_UPDATE_FILES) $(C_FORM_IN_FILES) $(C_DEBUG_FILES)
dev: clean_dev $(B_FILE_DEV)
	@echo ""

devf:
	@echo ""
	./scripts/make_devf

devformat: FLAGS   += $(FORMAT_FLAGS)
devformat: C_FILES += $(C_UPDATE_FILES) $(C_FORM_IN_FILES) $(C_DEBUG_FILES)
devformat: clean_dev $(B_FILE_DEV)

install:
	cp termfu /usr/bin/

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

help:
	@./scripts/make_help

build:
	(cd $(TEST_PROGS_DIR) && ./build_c)

run_dev:
	./$(B_FILE_DEV) -c $(CONFIG_RUN_DEV) -p $(DATA_RUN_DEV)

debug:
	$(B_FILE_PROD) -c $(CONFIG_DEBUG) -p $(DATA_DEBUG)

debug_gdb:
	gdb --quiet --tui --args ./termfu_dev -c $(CONFIG_DEBUGGED) -p $(DATA_DEBUGGED)

todo:
	./scripts/make_todo

plugins:
	./scripts/make_plugins

# DO NOT DELETE

src/choose_layout.o: /usr/include/string.h /usr/include/ncurses.h
src/choose_layout.o: /usr/include/ncurses_dll.h /usr/include/stdint.h
src/choose_layout.o: /usr/include/stdio.h /usr/include/unctrl.h
src/choose_layout.o: /usr/include/curses.h src/data.h /usr/include/time.h
src/choose_layout.o: /usr/include/features.h /usr/include/features-time64.h
src/choose_layout.o: /usr/include/stdc-predef.h /usr/include/pthread.h
src/choose_layout.o: /usr/include/sched.h src/choose_layout.h
src/choose_layout.o: src/render_layout.h
src/choose_layout.o: src/update_window_data/_update_window_data.h src/data.h
src/choose_layout.o: src/plugins.h src/utilities.h /usr/include/stdlib.h
src/choose_layout.o: /usr/include/alloca.h /usr/include/errno.h
src/choose_layout.o: src/parse_config_file.h
src/choose_layout.o: src/data.h /usr/include/ncurses.h
src/choose_layout.o: /usr/include/ncurses_dll.h /usr/include/stdint.h
src/choose_layout.o: /usr/include/stdio.h /usr/include/unctrl.h
src/choose_layout.o: /usr/include/curses.h /usr/include/time.h
src/choose_layout.o: /usr/include/features.h /usr/include/features-time64.h
src/choose_layout.o: /usr/include/stdc-predef.h /usr/include/pthread.h
src/choose_layout.o: /usr/include/sched.h
src/data.o: /usr/include/ncurses.h /usr/include/ncurses_dll.h
src/data.o: /usr/include/stdint.h /usr/include/stdio.h /usr/include/unctrl.h
src/data.o: /usr/include/curses.h /usr/include/time.h /usr/include/features.h
src/data.o: /usr/include/features-time64.h /usr/include/stdc-predef.h
src/data.o: /usr/include/pthread.h /usr/include/sched.h
src/display_lines.o: /usr/include/string.h /usr/include/ncurses.h
src/display_lines.o: /usr/include/ncurses_dll.h /usr/include/stdint.h
src/display_lines.o: /usr/include/stdio.h /usr/include/unctrl.h
src/display_lines.o: /usr/include/curses.h /usr/include/stdlib.h
src/display_lines.o: /usr/include/alloca.h /usr/include/features.h
src/display_lines.o: /usr/include/features-time64.h
src/display_lines.o: /usr/include/stdc-predef.h src/display_lines.h
src/display_lines.o: src/data.h /usr/include/time.h /usr/include/pthread.h
src/display_lines.o: /usr/include/sched.h src/utilities.h
src/display_lines.o: /usr/include/errno.h src/parse_config_file.h
src/display_lines.o: src/render_layout.h src/plugins.h
src/display_lines.o: src/format_window_data.h
src/display_lines.o: src/data.h /usr/include/ncurses.h
src/display_lines.o: /usr/include/ncurses_dll.h /usr/include/stdint.h
src/display_lines.o: /usr/include/stdio.h /usr/include/unctrl.h
src/display_lines.o: /usr/include/curses.h /usr/include/time.h
src/display_lines.o: /usr/include/features.h /usr/include/features-time64.h
src/display_lines.o: /usr/include/stdc-predef.h /usr/include/pthread.h
src/display_lines.o: /usr/include/sched.h
src/format_window_data.o: /usr/include/string.h /usr/include/stdlib.h
src/format_window_data.o: /usr/include/alloca.h /usr/include/features.h
src/format_window_data.o: /usr/include/features-time64.h
src/format_window_data.o: /usr/include/stdc-predef.h /usr/include/libgen.h
src/format_window_data.o: /usr/include/ctype.h src/data.h
src/format_window_data.o: /usr/include/ncurses.h /usr/include/ncurses_dll.h
src/format_window_data.o: /usr/include/stdint.h /usr/include/stdio.h
src/format_window_data.o: /usr/include/unctrl.h /usr/include/curses.h
src/format_window_data.o: /usr/include/time.h /usr/include/pthread.h
src/format_window_data.o: /usr/include/sched.h src/plugins.h
src/format_window_data.o: src/data.h /usr/include/ncurses.h
src/format_window_data.o: /usr/include/ncurses_dll.h /usr/include/stdint.h
src/format_window_data.o: /usr/include/stdio.h /usr/include/unctrl.h
src/format_window_data.o: /usr/include/curses.h /usr/include/time.h
src/format_window_data.o: /usr/include/features.h
src/format_window_data.o: /usr/include/features-time64.h
src/format_window_data.o: /usr/include/stdc-predef.h /usr/include/pthread.h
src/format_window_data.o: /usr/include/sched.h