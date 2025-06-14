#include "test_utilities.h"
#include "unistd.h"

#include "../src/data.h"
#include "../src/utilities.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_local_vars.h"

#define PLUG_IDX   LcV


void run_tests_pdb_get_break_data (state_t *state)
{
    window_t *win = state->plugins[PLUG_IDX]->win;

    send_command_mp (state, "break 10\n");
    send_command_mp (state, "continue\n");

    get_local_vars (state);
    printf ("Local vars: \n%s\n\n", win->buff_data->buff);
}

int main (int argc, char *argv[]) 
{
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    
    run_tests_pdb_get_break_data (state);

    return 0;
}

