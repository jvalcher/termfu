
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
NCURSES_CFLAGS = $(shell ncurses5-config --cflags)
NCURSES_LIBS   = $(shell ncurses5-config --libs) -lform

# source files
C_FILES         = $(wildcard ./src/*.c)
C_TEST_FILES    = $(filter-out ./src/main.c, $(wildcard ./src/*.c))
C_UPDATE_FILES  = $(wildcard ./src/update_window_data/*.c)
C_FORM_IN_FILES = $(wildcard ./src/get_form_input/*.c)
C_DEBUG_FILES   = $(wildcard ./src/debug/*.c)

# termfu configuration, data files
SCRIPT_CONFIG   = scripts/.termfu
CONFIG_RUN_GDB	= $(SCRIPT_CONFIG)_run_gdb
DATA_RUN_GDB	= $(SCRIPT_CONFIG)_run_gdb_data
CONFIG_RUN_PDB	= $(SCRIPT_CONFIG)_run_pdb
DATA_RUN_PDB	= $(SCRIPT_CONFIG)_run_pdb_data
CONFIG_DEBUG	= $(SCRIPT_CONFIG)_debug
DATA_DEBUG		= $(SCRIPT_CONFIG)_debug_data

.PHONY: help all dev devf devformat build_gdb run_gdb run_pdb plugins debug_proc_gdbtui debug_proc_termfu proc_gdb proc_pdb clean_prod clean_dev


all: FLAGS   += $(PROD_FLAGS)
all: C_FILES += $(C_UPDATE_FILES)
all: C_FILES += $(C_FORM_IN_FILES)
all: C_FILES += $(C_DEBUG_FILES)
all: clean_prod $(B_FILE_PROD)
	@echo ""

dev: FLAGS   += $(DEV_FLAGS)
dev: C_FILES += $(C_UPDATE_FILES)
dev: C_FILES += $(C_FORM_IN_FILES)
dev: C_FILES += $(C_DEBUG_FILES)
dev: clean_dev $(B_FILE_DEV)
	@echo ""

devf:
	@echo ""
	@./scripts/make_devf

devformat: FLAGS   += $(FORMAT_FLAGS)
devformat: C_FILES += $(C_UPDATE_FILES)
devformat: C_FILES += $(C_FORM_IN_FILES)
devformat: C_FILES += $(C_DEBUG_FILES)
devformat: clean_dev $(B_FILE_DEV)

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

build_gdb:
	(cd misc && ./build_hello)
	(cd misc && ./build_vars)
	(cd misc && ./build_fib)

run_gdb:
	@./$(B_FILE_DEV) -c $(CONFIG_RUN_GDB) -p $(DATA_RUN_GDB)

run_pdb:
	@./$(B_FILE_DEV) -c $(CONFIG_RUN_PDB) -p $(DATA_RUN_PDB)

debug:
	@./$(B_FILE_PROD) -c $(CONFIG_DEBUG) -p $(DATA_DEBUG)

todo:
	@./scripts/make_todo

plugins:
	@./scripts/make_plugins

