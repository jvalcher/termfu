#include "test_utilities.h"
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/utilities.h"
#include "../src/update_window_data/get_register_data.h"

#define PLUG_IDX  Reg


void run_tests_gdb_reg (state_t *state)
{
    char *reg_buff = state->plugins[PLUG_IDX]->win->buff_data->buff;

    send_command_mp (state, "-break-insert 12\n");
    send_command_mp (state, "-exec-run\n");

    get_register_data (state);
    printf ("%s\n", reg_buff);
}

int main (int argc, char *argv[]) 
{
    state_t *state = start_termfu(argc, argv);
    deinit_nc();
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    
    run_tests_gdb_reg (state);

    clean_up (PROG_EXIT);
    return 0;
}

