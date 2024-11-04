
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
CONFIG_RUN_DEV = configs/.termfu_run_dev
DATA_RUN_DEV   = configs/.termfu_run_dev_data
CONFIG_DEBUG   = configs/.termfu_debugger
DATA_DEBUG	   = configs/.termfu_debugger_data

.PHONY: help all install dev devf devformat build run_dev plugins clean_prod clean_dev


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
	(cd $(TEST_PROGS_DIR) && ./build_hello && ./build_vars && ./build_fib)

run_dev:
	./$(B_FILE_DEV) -c $(CONFIG_RUN_DEV) -p $(DATA_RUN_DEV)

debug:
	$(B_FILE_PROD) -c $(CONFIG_DEBUG) -p $(DATA_DEBUG)

todo:
	./scripts/make_todo

plugins:
	./scripts/make_plugins

