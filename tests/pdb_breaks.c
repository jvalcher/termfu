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
    breakpoint_t *curr_break;
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Brk] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Brk]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Brk]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Brk_BUF_LEN);

    win->buff_data->buff_len = Brk_BUF_LEN;
    state->debugger->index = DEBUGGER_PDB;
    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;

    start_debugger (state);

    // no breakpoints
    get_breakpoint_data (state);

    putchar ('\n');
    printf ("No breakpoints: \"%s\"\n\n", win->buff_data->buff);

    // breakpoints
    insert_output_start_marker (state);
    send_command (state, "break 10\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "break gcd2.py:4\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "break gcd.py:14\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_breakpoint_data (state);

    printf ("Breakpoints: \n\"%s\"\n\n", win->buff_data->buff);

    // breakpoint_t
    curr_break = state->breakpoints;
    do {
        printf ("breakpoint_t: %s\n", curr_break->path_line);
        curr_break = curr_break->next;
    } while (curr_break != NULL);
    putchar ('\n');

    return 0;
}

