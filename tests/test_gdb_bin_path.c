#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "test_utilities.h"
#include "utest.h"
#include "../src/data.h"
#include "../src/error.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/update_window_data/get_binary_path_time.h"
#include "../src/utilities.h"

#define TEST_BIN  "test_programs/hello"     // Matches ../configs/.termfu_test

int tests_passed = 0,
    tests_failed = 0;


void run_tests_gdb_bin_path (state_t *state)
{
    char *abs_path;
    char *prog_path = state->debugger->prog_path;

    test_init();

    // Absolute paths equal
    abs_path = realpath(TEST_BIN, NULL);
    test_strs_eq (abs_path, prog_path, "\"%s\" != \"%s\"", abs_path, prog_path);
    free(abs_path);

    test_results();
}


int main (int argc, char *argv[]) 
{
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    usleep (50000);
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    
    run_tests_gdb_bin_path (state);

    clean_up (PROG_EXIT);
    return 0;
}

