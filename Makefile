#
#  $ make help
#

# make
MAKEFLAGS += --no-print-directory

# binaries
B_FILE_PROD    = termfu
B_FILE_DEV     = termfu_dev
B_FILE_TEST	   = termfu_test

PROD_DIR	   = /usr/local/bin

# Flags
CC             = gcc
FLAGS          = -MMD -Wall -Wextra -Werror -pedantic-errors
PROD_FLAGS     = -O3
DEV_FLAGS      = -g
FORMAT_FLAGS   = -fdiagnostics-format=json
NCURSES_CFLAGS = $(shell ncurses6-config --cflags)
NCURSES_LIBS   = $(shell ncurses6-config --libs) -lform

# source files, directories
	# directories
C_DIR          = src
C_DIR_UPDATE   = $(C_DIR)/update_window_data
C_DIR_FORM_IN  = $(C_DIR)/get_form_input
TESTS_DIR	   = tests
TEST_PROGS_DIR = test_programs
	# files
C_FILES          = $(wildcard $(C_DIR)/*.c)
C_FILES_TEST     = $(filter-out $(C_DIR)/main.c, $(wildcard $(C_DIR)/*.c))
TEST_UTIL_FILE	 = $(TESTS_DIR)/test_utilities.c
C_UPDATE_FILES   = $(wildcard $(C_DIR_UPDATE)/*.c)
C_FORM_IN_FILES  = $(wildcard $(C_DIR_FORM_IN)/*.c)
C_FILES         += $(C_UPDATE_FILES) $(C_FORM_IN_FILES)
C_FILES_TEST    += $(C_UPDATE_FILES) $(C_FORM_IN_FILES)

# object files, directories
DEV_OBJ_DIR        = obj/dev
PROD_OBJ_DIR       = obj/prod
OBJ_FILES          = $(notdir $(C_FILES:%.c=%.o))
OBJ_FILES_TEST     = $(notdir $(C_FILES_TEST:%.c=%.o))
DEV_OBJ_FILES      = $(OBJ_FILES:%.o=$(DEV_OBJ_DIR)/%.o)
TEST_OBJ_FILE      = $(DEV_OBJ_DIR)/$(notdir $(t:%.c=%.o))
TEST_UTIL_OBJ_FILE = $(DEV_OBJ_DIR)/$(notdir $(TEST_UTIL_FILE:%.c=%.o))
TEST_OBJ_FILES     = $(OBJ_FILES_TEST:%.o=$(DEV_OBJ_DIR)/%.o)
PROD_OBJ_FILES     = $(OBJ_FILES:%.o=$(PROD_OBJ_DIR)/%.o)

# dependency files
DEP_FILES      = $(OBJ_FILES:%.o=%.d)

# configuration, data files
CONFIG_DIR        = configs
CONFIG_RUN_DEV    = $(CONFIG_DIR)/.termfu_run_dev
DATA_RUN_DEV      = $(CONFIG_DIR)/.termfu_run_dev_data
CONFIG_TEST_GDB   = $(CONFIG_DIR)/.termfu_test_gdb
DATA_TEST_GDB 	  = $(CONFIG_DIR)/.termfu_test_gdb_data
CONFIG_TEST_PDB   = $(CONFIG_DIR)/.termfu_test_pdb
DATA_TEST_PDB 	  = $(CONFIG_DIR)/.termfu_test_pdb_data
CONFIG_DEBUG_DEV  = $(CONFIG_DIR)/.termfu_debugger_dev
DATA_DEBUG_DEV	  = $(CONFIG_DIR)/.termfu_debugger_dev_data
CONFIG_DEBUG_TEST = $(CONFIG_DIR)/.termfu_debugger_test
DATA_DEBUG_TEST	  = $(CONFIG_DIR)/.termfu_debugger_test_data
CONFIG_DEBUGGED   = $(CONFIG_DIR)/.termfu_debugged
DATA_DEBUGGED     = $(CONFIG_DIR)/.termfu_debugged_data

SCRIPTS_DIR	= scripts

-include $(DEP_FILES)

#
# Production build
#
.PHONY: all
all: FLAGS += $(PROD_FLAGS)
all: clean_prod $(B_FILE_PROD)
$(B_FILE_PROD): $(PROD_OBJ_FILES)
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
dev: FLAGS += $(DEV_FLAGS)
dev: $(B_FILE_DEV)
$(B_FILE_DEV): $(DEV_OBJ_FILES)
	@echo ""
	$(CC) $(DEV_OBJ_FILES) -o $(BIN_DEV) $(NCURSES_LIBS)
$(DEV_OBJ_DIR)/%.o : $(C_DIR)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_UPDATE)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_FORM_IN)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@

#
# Test build, run
#
.PHONY: test_gdb
test_gdb: FLAGS += $(DEV_FLAGS)
test_gdb: $(B_FILE_TEST) 
	@printf "\n###\nRunning \"%s\" ...\n###\n\n" $(t)
	./$(B_FILE_TEST) -c $(CONFIG_TEST_GDB) -p $(DATA_TEST_GDB)
	@printf ""

.PHONY: test_pdb
test_pdb: FLAGS += $(DEV_FLAGS)
test_pdb: $(B_FILE_TEST) 
	@printf "\n###\nRunning \"%s\" ...\n###\n\n" $(t)
	./$(B_FILE_TEST) -c $(CONFIG_TEST_PDB) -p $(DATA_TEST_PDB)
	@printf ""

.PHONY: test
test: FLAGS += $(DEV_FLAGS)
test: $(B_FILE_TEST) 
$(B_FILE_TEST): $(TEST_OBJ_FILES) $(TEST_OBJ_FILE)
	$(CC) $(TEST_OBJ_FILES) $(TEST_OBJ_FILE) $(TEST_UTIL_OBJ_FILE) -o $(B_FILE_TEST) $(NCURSES_LIBS)
$(TEST_OBJ_FILE): $(t) $(TEST_OBJ_FILES) $(TEST_UTIL_OBJ_FILE)
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $(t) -o $(TEST_OBJ_FILE)
$(TEST_UTIL_OBJ_FILE): check_test_file $(TEST_OBJ_FILES) $(TEST_UTIL_FILE)
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $(TEST_UTIL_FILE) -o $(TEST_UTIL_OBJ_FILE)
$(DEV_OBJ_DIR)/%.o : $(C_DIR)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_UPDATE)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@
$(DEV_OBJ_DIR)/%.o : $(C_DIR_FORM_IN)/%.c
	$(CC) $(FLAGS) $(NCURSES_CFLAGS) -c $< -o $@

.PHONY: check_test_file
check_test_file:
ifeq ($(t), $(TEST_UTIL_FILE))
	$(error test_utilities.c is not a test file)
endif
ifndef t
	$(error You must specify the test file with t=..., e.g. make test t=tests/my_test.c)
endif


#
# Misc scripts
#
.PHONY: install
install:
	cp termfu $(PROD_DIR)

.PHONY: devf
devf:
	./$(SCRIPTS_DIR)/make_devf

.PHONY: devformat
devformat: FLAGS += $(DEV_FLAGS)
devformat: FLAGS += $(FORMAT_FLAGS)
devformat: $(B_FILE_DEV)

.PHONY: clean_prod
clean_prod:
	rm -f $(PROD_OBJ_DIR)/*
	rm -f $(B_FILE_PROD)

.PHONY: clean_dev
clean_dev:
	rm -f $(DEV_OBJ_DIR)/*
	rm -f $(B_FILE_DEV)

.PHONY: help
help:
	./$(SCRIPTS_DIR)/make_help

.PHONY: build_tests
build_tests:
	(cd $(TEST_PROGS_DIR) && ./build_c)

.PHONY: run_dev
run_dev:
	./$(BIN_DEV) -c $(CONFIG_RUN_DEV) -p $(DATA_RUN_DEV)

.PHONY: debug_dev
debug_dev:
	$(B_FILE_PROD) -c $(CONFIG_DEBUG_DEV) -p $(DATA_DEBUG_DEV)

.PHONY: debug_test
debug_test:
	$(B_FILE_PROD) -c $(CONFIG_DEBUG_TEST) -p $(DATA_DEBUG_TEST)

.PHONY: debug_dev_gdb
debug_dev_gdb:
	gdb --quiet --tui --args $(B_FILE_DEV) -c $(CONFIG_DEBUGGED) -p $(DATA_DEBUGGED)

.PHONY: debug_test_gdb
debug_test_gdb:
	gdb --quiet --tui --args $(B_FILE_TEST) -c $(CONFIG_DEBUGGED) -p $(DATA_DEBUGGED)

.PHONY: debugged
debugged:
	./$(SCRIPTS_DIR)/make_debugged

.PHONY: todo
todo:
	./$(SCRIPTS_DIR)/make_todo

.PHONY: plugins
plugins:
	./$(SCRIPTS_DIR)/make_plugins

