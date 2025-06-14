#include "test_utilities.h"
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/utilities.h"
#include "../src/display_lines.h"
#include "../src/get_form_input/watchpoints.h"
#include "../src/update_window_data/get_watchpoint_data.h"

#define PLUG_IDX  Wat

int row = 2;



void print_watchpoints (state_t *state)
{
    watchpoint_t *wps = state->watchpoints;
    
    move(row, 0);
    while (wps) {
        printw ("[%d] \"%s\": \"%s\"  -->  ", wps->index, wps->var, wps->value);
        wps = wps->next;
    }
    printw ("NULL\n\n");
    row += 2;
    refresh();
}


void run_test_pdb_watch (state_t *state)
{
    printw ("Program: \"%s\"  (num1, num2, narr, sarr, str1, total)\n\n", 
                state->debugger->prog_path);
    refresh();

    send_command_mp (state, "break 10\n");
    send_command_mp (state, "-exec-run\n");

    insert_watchpoint (state);
    get_watchpoint_data (state);
    print_watchpoints (state);

    insert_watchpoint (state);
    get_watchpoint_data (state);
    print_watchpoints (state);

    insert_watchpoint (state);
    get_watchpoint_data (state);
    print_watchpoints (state);

    delete_watchpoint (state);
    get_watchpoint_data (state);
    print_watchpoints (state);

    delete_watchpoint (state);
    get_watchpoint_data (state);
    print_watchpoints (state);

    move(row, 0);
    printw ("Press any key to continue...\n");
    refresh();
    getch();
}

int main (int argc, char *argv[]) 
{
    printf ("\n\n###\nTEST BEGIN...\n###\n\n");
    state_t *state = start_termfu(argc, argv);

    run_test_pdb_watch (state);

    deinit_nc();
    return 0;
}


