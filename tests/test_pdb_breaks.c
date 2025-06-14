#include "test_utilities.h"
#include "unistd.h"

#include "../src/data.h"
#include "../src/utilities.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_breakpoint_data.h"

#define PLUG_IDX   Brk


void run_tests_pdb_get_break_data (state_t *state)
{
    char *brk_buff = state->plugins[PLUG_IDX]->win->buff_data->buff;

    send_command_mp (state, "break 12\n");
    send_command_mp (state, "break gcd.py:17\n");
    send_command_mp (state, "break gcd2.py:4\n");

    get_breakpoint_data (state);
    printf ("\nBreakpoing buffer len: %lu \n\n%s\n\n", strlen(brk_buff), brk_buff);
}

int main (int argc, char *argv[]) 
{
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    
    run_tests_pdb_get_break_data (state);

    return 0;
}

