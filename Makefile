
#
#   -------
#   make      		   - Build production binary
#   make dev		   - Build development binary
#   make devf   	   - Build development binary, print formatted error messages
#   make configs       - Create all testing, debugging configuration files
#   make run_dev_gdb   - Run test gdb session
#   make run_dev_pdb   - Run test pdb session
#
#   make todo          - Print source code tags in source code (TODO, FIXME, etc.)
#   make tui_gdb       - Run Tmux GDB TUI debugging session on GDB session
#   make tui_pdb       - Run Tmux GDB TUI debugging session on PDB session
#   make target        - Start termfu_dev and target one of the below running gdbservers
#	make server_gdb    - Start termfu_dev gdbserver with gdb configuration
#	make server_pdb    - Start termfu_dev gdbserver with pdb configuration
#   -------
#


# binaries
B_FILE_PROD    = termfu
B_FILE_DEV     = termfu_dev

# flags
CC             = gcc
FLAGS          = -Wall -Wextra
PROD_FLAGS     = -O3
DEV_FLAGS      = -g
FORMAT_FLAGS   = -g -fdiagnostics-format=json
NCURSES_CFLAGS = $(shell ncurses5-config --cflags)
NCURSES_LIBS   = $(shell ncurses5-config --libs)

# source files
C_FILES        = $(wildcard ./src/*.c)
C_TEST_FILES   = $(filter-out ./src/main.c, $(wildcard ./src/*.c))
C_UPDATE_FILES = $(wildcard ./src/update_window_data/*.c)
C_POPUP_FILES  = $(wildcard ./src/get_popup_window_input/*.c)

# configuration, data files  (see scripts/create_configs)
CONFIG_BASE    = scripts/.termfu
CONFIG_RUN_GDB = $(CONFIG_BASE)_run_gdb
CONFIG_RUN_PDB = $(CONFIG_BASE)_run_pdb
DATA_RUN_GDB   = $(CONFIG_RUN_GDB)_data
DATA_RUN_PDB   = $(CONFIG_RUN_PDB)_data


.PHONY: all dev devf devformat congigs run_dev_gdb run_dev_pdb todo tui_gdb tui_pdb target server_gdb server_pdb clean_prod clean_dev


all: FLAGS   += $(PROD_FLAGS)
all: C_FILES += $(C_UPDATE_FILES)
all: C_FILES += $(C_POPUP_FILES)
all: clean_prod $(B_FILE_PROD)
	@echo ""

dev: FLAGS   += $(DEV_FLAGS)
dev: C_FILES += $(C_UPDATE_FILES)
dev: C_FILES += $(C_POPUP_FILES)
dev: clean_dev $(B_FILE_DEV)
	@echo ""

devf:
	@echo ""
	@./scripts/make_format_dev

devformat: FLAGS   += $(FORMAT_FLAGS)
devformat: C_FILES += $(C_UPDATE_FILES)
devformat: C_FILES += $(C_POPUP_FILES)
devformat: clean_dev $(B_FILE_DEV)

run_dev_gdb:
	@echo ""
	(cd misc && ./build_hello) && ./$(B_FILE_DEV) -c $(CONFIG_RUN_GDB) -d $(DATA_RUN_GDB)
	@echo ""

run_dev_pdb:
	@echo ""
	./$(B_FILE_DEV) -c $(CONFIG_RUN_PDB) -d $(DATA_RUN_PDB)
	@echo ""

tui_gdb:
	@echo ""
	./scripts/tui_debug_gdb
	@echo ""

tui_pdb:
	@echo ""
	./scripts/tui_debug_pdb
	@echo ""

target:
	@echo ""
	./scripts/gdb_target_server
	@echo ""

server_gdb:
	@echo ""
	./scripts/gdbserver_termfu_gdb
	@echo ""

server_pdb:
	@echo ""
	./scripts/gdbserver_termfu_pdb
	@echo ""

configs:
	@echo ""
	./scripts/create_configs
	@echo ""

todo:
	@echo ""
	./scripts/todo
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

