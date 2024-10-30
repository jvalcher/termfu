#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_breakpoint_data.h"
#include "../src/plugins.h"


int
main (void)
{
    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Brk;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    //buff_data_t *buff_data = win->buff_data;

    ////////////

    breakpoint_t *curr_break;

    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;
    debugger->index = DEBUGGER_PDB;
    start_debugger (state);

    // no breakpoints
    get_breakpoint_data (state);

    putchar ('\n');
    printf ("No breakpoints: \"%s\"\n\n", win->buff_data->buff);

    // breakpoints
    send_command_mp (state, "break 10\n");
    send_command_mp (state, "break gcd2.py:4\n");
    send_command_mp (state, "break gcd.py:14\n");

    get_breakpoint_data (state);

    printf ("Breakpoints: \n\"%s\"\n\n", win->buff_data->buff);

    // breakpoint_t
    curr_break = state->breakpoints;
    do {
        printf ("breakpoint_t: (%s) \"%s\"\n", curr_break->line, curr_break->path);
        curr_break = curr_break->next;
    } while (curr_break != NULL);
    putchar ('\n');

    return 0;
}

