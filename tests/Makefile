
BUILD    = gcc -g -Wextra
TEST_BIN = test

SRC_DIR  = ../src
UTILS_SRC = $(SRC_DIR)/utils.c
DEBUG_SRC = $(SRC_DIR)/debugger.c
DATA_SRC = $(SRC_DIR)/data.c

TEST_PROG_DIR  = test_programs
TEST_PROGS     = hello fib infinite_loop seg_fault vars
TEST_PROGS    := $(addprefix $(TEST_PROG_DIR)/, $(TEST_PROGS))

#
# Tests
#

.PHONY: all
all: data debugger utils
all: $(TEST_PROGS)

.PHONY: data
data: $(TEST_PROGS)
	@rm -f $(TEST_BIN)
	@$(BUILD) $(DATA_SRC) $(DEBUG_SRC) $(UTILS_SRC) test_data.c -o $(TEST_BIN)
	@./$(TEST_BIN)

.PHONY: debugger
debugger: $(TEST_PROGS)
	@rm -f $(TEST_BIN)
	@$(BUILD) test_debugger.c $(DEBUG_SRC) $(UTILS_SRC) -o $(TEST_BIN)
	@./$(TEST_BIN)

.PHONY: utils
utils: $(TEST_PROGS)
	@rm -f $(TEST_BIN)
	@$(BUILD) test_utils.c $(UTILS_SRC) -o $(TEST_BIN)
	@./$(TEST_BIN)

#
# Test programs
#

.PHONY: test_programs
test_programs:
	$(BUILD) $(TEST_PROG_DIR)/hello.c $(TEST_PROG_DIR)/hello2.c -o $(TEST_PROG_DIR)/hello
	$(BUILD) $(TEST_PROG_DIR)/fib.c -o $(TEST_PROG_DIR)/fib
	$(BUILD) $(TEST_PROG_DIR)/infinite_loop.c $(TEST_PROG_DIR)/infinite_loop_func.c -o $(TEST_PROG_DIR)/infinite_loop
	$(BUILD) $(TEST_PROG_DIR)/seg_fault.c -o $(TEST_PROG_DIR)/seg_fault
	$(BUILD) $(TEST_PROG_DIR)/vars.c -o $(TEST_PROG_DIR)/vars
	$(BUILD) $(TEST_PROG_DIR)/threads.c -o $(TEST_PROG_DIR)/threads
