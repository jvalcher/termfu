
#
# $ make help
#


B_FILE         = a.out
CC             = gcc
FLAGS          = -g -Wall -Wextra

NCURSES_CFLAGS = $(shell ncurses5-config --cflags)
NCURSES_LIBS   = $(shell ncurses5-config --libs)

C_FILES        = $(filter-out ../src/main.c, $(wildcard ../src/*.c))
C_UPDATE_FILES = $(wildcard ../src/update_window_data/*.c)
C_POPUP_FILES  = $(wildcard ../src/get_popup_window_input/*.c)


.PHONY: all help debug clean

all: C_FILES += $(t)
all: C_FILES += $(C_UPDATE_FILES)
all: C_FILES += $(C_POPUP_FILES)
all: clean $(B_FILE)
	@./$(B_FILE)
	@echo ""

help:
	@../scripts/make_help_tests

debug:
	@echo ""
	@../scripts/debug_test
	@echo ""

clean:
	@echo ""
	@rm -f $(B_FILE)
	@echo ""

$(B_FILE):
	@$(CC) $(FLAGS) $(NCURSES_CFLAGS) $(C_FILES) -o $(B_FILE) $(NCURSES_LIBS)
	@echo ""

