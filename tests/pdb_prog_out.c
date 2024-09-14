#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_program_output.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Prg] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Prg]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Prg]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    window_t *win = state->plugins[Prg]->win;
    win->buff_data->buff = (char*) malloc (sizeof (char) * Prg_BUF_LEN);

    win->buff_data->buff_len = Prg_BUF_LEN;
    state->debugger->index = DEBUGGER_PDB;
    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;

    start_debugger (state);

    win->buff_data->new_data = true;
    insert_output_start_marker (state);
    send_command (state, "break 11\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    win->buff_data->new_data = true;
    insert_output_start_marker (state);
    send_command (state, "continue\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    win->buff_data->new_data = true;
    insert_output_start_marker (state);
    send_command (state, "next\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_program_output (state);
    printf ("out: \n\"%s\"\n\n", win->buff_data->buff);
    
    win->buff_data->new_data = true;
    insert_output_start_marker (state);
    send_command (state, "next\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_program_output (state);
    printf ("out: \n\"%s\"\n\n", win->buff_data->buff);
    
    return 0;
}


