#include "test_utilities.h"
#include "unistd.h"

#include "../src/data.h"
#include "../src/utilities.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_source_path_line_func.h"

#define PLUG_IDX   Src


void run_tests_pdb_get_break_data (state_t *state)
{
    send_command_mp (state, "break 10\n");
    send_command_mp (state, "continue\n");

    get_source_path_line_func (state);
    printf ("%s (%d)\n\n", state->debugger->src_path_buffer, state->debugger->curr_Src_line);
}

int main (int argc, char *argv[]) 
{
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    
    run_tests_pdb_get_break_data (state);

    return 0;
}

