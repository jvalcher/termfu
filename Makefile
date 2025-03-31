#
#  $ make help
#

# make
MAKEFLAGS += --no-print-directory

# Binaries
BIN_PROD    = termfu
BIN_DEV     = termfu_dev

# Flags
CC             = gcc
FLAGS          = -MMD -Wall -Wextra -Werror -pedantic-errors
PROD_FLAGS     = -O3
DEV_FLAGS      = -DDEV -g
FORMAT_FLAGS   = $(DEV_FLAGS) -fdiagnostics-format=json
NCURSES_CFLAGS = $(shell ncurses6-config --cflags)
NCURSES_LIBS   = $(shell ncurses6-config --libs) -lform

# Source files, directories
	# Directories
C_DIR          = ./src
C_DIR_UPDATE   = $(C_DIR)/update_window_data
C_DIR_FORM_IN  = $(C_DIR)/get_form_input
TEST_PROGS_DIR = ./scripts/test_programs
	# Files
C_FILES          = $(wildcard $(C_DIR)/*.c)
C_UPDATE_FILES   = $(wildcard $(C_DIR_UPDATE)/*.c)
C_FORM_IN_FILES  = $(wildcard $(C_DIR_FORM_IN)/*.c)
C_FILES         += $(C_UPDATE_FILES) $(C_FORM_IN_FILES)

# Object files, directories
OBJ_FILES      = $(notdir $(C_FILES:%.c=%.o))
DEV_OBJ_DIR    = obj/dev
PROD_OBJ_DIR   = obj/prod
DEV_OBJ_FILES  = $(OBJ_FILES:%.o=$(DEV_OBJ_DIR)/%.o)
PROD_OBJ_FILES = $(OBJ_FILES:%.o=$(PROD_OBJ_DIR)/%.o)

# Configuration, data files
CONFIG_RUN_DEV  = configs/.termfu_run_dev
DATA_RUN_DEV    = configs/.termfu_run_dev_data
CONFIG_DEBUG    = configs/.termfu_debugger
DATA_DEBUG	    = configs/.termfu_debugger_data
CONFIG_DEBUGGED = configs/.termfu_debugged
DATA_DEBUGGED   = configs/.termfu_debugged_data

# Dependency files
DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

#
# Production build
#

.PHONY: all
all: FLAGS   += $(PROD_FLAGS)
all: clean_prod $(BIN_PROD)

$(BIN_PROD): $(PROD_OBJ_FILES)
	@echo ""
	$(CC) $(PROD_OBJ_FILES) -o $(BIN_PROD) $(NCURSES_LIBS)
$(PROD_OBJ_DIR)/%.o : $(C_DIR)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(PROD_OBJ_DIR)/%.o : $(C_DIR_UPDATE)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(PROD_OBJ_DIR)/%.o : $(C_DIR_FORM_IN)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@

#
# Development build
#

.PHONY: dev
dev: FLAGS   += $(DEV_FLAGS)
dev: $(BIN_DEV)

$(BIN_DEV): $(DEV_OBJ_FILES)
	@echo ""
	$(CC) $(DEV_OBJ_FILES) -o $(BIN_DEV) $(NCURSES_LIBS)
$(DEV_OBJ_DIR)/%.o : $(C_DIR)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_UPDATE)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_FORM_IN)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@

#
# Misc scripts
#

.PHONY: help
help:
	./scripts/make_help

.PHONY: install
install:
	cp termfu /usr/local/bin

.PHONY: devf
devf:
	./scripts/make_devf

.PHONY: devformat
devformat: FLAGS += $(FORMAT_FLAGS)
devformat: $(BIN_DEV)

.PHONY: clean_prod
clean_prod:
	rm -f $(PROD_OBJ_DIR)/*
	rm -f $(BIN_PROD)

.PHONY: clean_dev
clean_dev:
	rm -f $(DEV_OBJ_DIR)/*
	rm -f $(BIN_DEV)

.PHONY: build_tests
build_tests:
	(cd $(TEST_PROGS_DIR) && ./build_c)

.PHONY: run_dev
run_dev:
	./$(BIN_DEV) -c $(CONFIG_RUN_DEV) -p $(DATA_RUN_DEV)

.PHONY: debug
debug:
	$(BIN_PROD) -c $(CONFIG_DEBUG) -p $(DATA_DEBUG)

.PHONY: debug_gdb
debug_gdb:
	gdb --quiet --tui --args ./$(BIN_DEV) -c $(CONFIG_DEBUG) -p $(DATA_DEBUG)

.PHONY: debugged
debugged:
	./scripts/make_debugged

.PHONY: todo
todo:
	./scripts/make_todo

.PHONY: plugins
plugins:
	./scripts/make_plugins

