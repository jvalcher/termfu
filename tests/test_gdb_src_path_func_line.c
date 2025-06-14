#include "test_utilities.h"
#include "../src/data.h"
#include "../src/utilities.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_source_path_line_func.h"

#define PLUG_IDX   Src


void run_tests_gdb_src_path_func_line (state_t *state)
{
    send_command_mp (state, "-break-insert hello2.c:9\n");
    send_command_mp (state, "-exec-run\n");
    get_source_path_line_func (state);

    printf ("Source path: \"%s\"\n", debugger->src_path_buffer);
    printf ("Source line: %d\n", debugger->curr_Src_line);
    printf ("Function: \"%s\"\n\n", debugger->curr_func);
}

int main (int argc, char *argv[]) 
{
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    debugger_t *debugger = state->debugger;
    
    run_tests_gdb_src_path_func_line (state);

    clean_up (PROG_EXIT);
    return 0;
}

