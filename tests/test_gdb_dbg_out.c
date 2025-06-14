#include "test_utilities.h"
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/utilities.h"
#include "../src/update_window_data/get_debugger_output.h"

#define PLUG_IDX   Src


void run_tests_gdb_dbg_out (state_t *state)
{
    window_t *win = state->plugins[PLUG_IDX]->win;
    char *cli_buff = state->debugger->cli_buffer;
    char *dbg_out_buff = state->plugins[Dbg]->win->buff_data->buff;

    send_command_mp (state, "-break-insert main\n");
    
    send_command_mp (state, "-exec-run\n");
    printf ("COMMAND -exec-run CLI: \n\n%s\n\n", cli_buff);
    win->buff_data->new_data = true;
    get_debugger_output (state);
    printf ("DEBUG OUT: \n\n%s\n\n", dbg_out_buff);
}

int main (int argc, char *argv[]) 
{
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    
    run_tests_gdb_dbg_out (state);

    clean_up (PROG_EXIT);
    return 0;
}

