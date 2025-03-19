
#
# $ make help
#


B_FILE          = a.out
CC              = gcc
FLAGS           = -g -Wall -Wextra

NCURSES_CFLAGS  = $(shell ncurses6-config --cflags)
NCURSES_LIBS    = $(shell ncurses6-config --libs) -lform

C_FILES         = $(filter-out ../src/main.c, $(wildcard ../src/*.c))
C_UPDATE_FILES  = $(wildcard ../src/update_window_data/*.c)
C_FORM_IN_FILES = $(wildcard ../src/get_form_input/*.c)
C_DEBUG_FILES   = $(wildcard ../src/debug/*.c)

.PHONY: all help debug clean

all: C_FILES += $(t)
all: C_FILES += $(C_UPDATE_FILES)
all: C_FILES += $(C_FORM_IN_FILES)
all: C_FILES += $(C_DEBUG_FILES)
all: clean $(B_FILE)
	@./$(B_FILE)

help:
	@../scripts/make_help_tests

debug:
	@../scripts/debug_test

clean:
	@rm -f $(B_FILE)

$(B_FILE):
	@$(CC) $(FLAGS) $(NCURSES_CFLAGS) $(C_FILES) test_utilities.c -o $(B_FILE) $(NCURSES_LIBS)

