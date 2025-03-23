
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
FLAGS          = -MMD -Wall -Wextra -Werror -pedantic-errors
PROD_FLAGS     = -O3
DEV_FLAGS      = -g
FORMAT_FLAGS   = $(DEV_FLAGS) -fdiagnostics-format=json
NCURSES_CFLAGS = $(shell ncurses6-config --cflags)
NCURSES_LIBS   = $(shell ncurses6-config --libs) -lform

# source files, directories
	# directories
C_DIR          = ./src
C_DIR_UPDATE   = $(C_DIR)/update_window_data
C_DIR_FORM_IN  = $(C_DIR)/get_form_input
TEST_PROGS_DIR = ./test_programs
	# files
C_FILES          = $(wildcard $(C_DIR)/*.c)
C_UPDATE_FILES   = $(wildcard $(C_DIR_UPDATE)/*.c)
C_FORM_IN_FILES  = $(wildcard $(C_DIR_FORM_IN)/*.c)
C_FILES         += $(C_UPDATE_FILES) $(C_FORM_IN_FILES)

# object files, directories
OBJ_FILES      = $(notdir $(C_FILES:%.c=%.o))
DEV_OBJ_DIR    = obj/dev
PROD_OBJ_DIR   = obj/prod
DEV_OBJ_FILES  = $(OBJ_FILES:%.o=$(DEV_OBJ_DIR)/%.o)
PROD_OBJ_FILES = $(OBJ_FILES:%.o=$(PROD_OBJ_DIR)/%.o)

# dependency files
DEP_FILES = $(OBJ_FILES:%.o=%.d)

# configuration, data files
CONFIG_RUN_DEV  = configs/.termfu_run_dev
DATA_RUN_DEV    = configs/.termfu_run_dev_data
CONFIG_DEBUG    = configs/.termfu_debugger
DATA_DEBUG	    = configs/.termfu_debugger_data
CONFIG_DEBUGGED = configs/.termfu_debugged
DATA_DEBUGGED   = configs/.termfu_debugged_data

.PHONY: help all install dev devf devformat debug debug_gdb build run_dev plugins clean_prod clean_dev


-include $(DEP_FILES)


#
# Production build
#
all: FLAGS   += $(PROD_FLAGS)
all: clean_prod $(B_FILE_PROD)

$(B_FILE_PROD): $(PROD_OBJ_FILES)
	@echo ""
	$(CC) $(PROD_OBJ_FILES) -o $(B_FILE_PROD) $(NCURSES_LIBS)
$(PROD_OBJ_DIR)/%.o : $(C_DIR)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(PROD_OBJ_DIR)/%.o : $(C_DIR_UPDATE)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(PROD_OBJ_DIR)/%.o : $(C_DIR_FORM_IN)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@


#
# Development build
#
dev: FLAGS   += $(DEV_FLAGS)
dev: $(B_FILE_DEV)

$(B_FILE_DEV): $(DEV_OBJ_FILES)
	@echo ""
	$(CC) $(DEV_OBJ_FILES) -o $(B_FILE_DEV) $(NCURSES_LIBS)
$(DEV_OBJ_DIR)/%.o : $(C_DIR)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_UPDATE)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_FORM_IN)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@


#
# Misc scripts
#
install:
	cp termfu /usr/local/bin

devf:
	./scripts/make_devf

devformat: FLAGS += $(FORMAT_FLAGS)
devformat: $(B_FILE_DEV)

clean_prod:
	rm -f $(PROD_OBJ_DIR)/*
	rm -f $(B_FILE_PROD)

clean_dev:
	rm -f $(DEV_OBJ_DIR)/*
	rm -f $(B_FILE_DEV)

help:
	./scripts/make_help

build_tests:
	(cd $(TEST_PROGS_DIR) && ./build_c)

run_dev:
	./$(B_FILE_DEV) -c $(CONFIG_RUN_DEV) -p $(DATA_RUN_DEV)

debug:
	$(B_FILE_PROD) -c $(CONFIG_DEBUG) -p $(DATA_DEBUG)

debug_gdb:
	gdb --quiet --tui --args ./termfu_dev -c $(CONFIG_DEBUGGED) -p $(DATA_DEBUGGED)

debugged:
	./scripts/make_debugged

todo:
	./scripts/make_todo

plugins:
	./scripts/make_plugins

