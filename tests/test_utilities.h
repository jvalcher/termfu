
#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include <stdlib.h>
#include "../src/data.h"

#define START_TEST(...) printf ("\033[1;33mSTART TEST: %s\033[1;0m\n", __VA_ARGS__)
#define END_TEST(...)   printf ("\033[1;35mEND TEST: %s\033[1;0m\n", __VA_ARGS__)

#define TEST_PASSED  printf ("\033[1;32mPASSED\033[1;0m\n")
#define TEST_FAILED  printf ("\033[1;31mFAILED\033[1;0m\n")



void print_buffers (state_t *state);



#endif
