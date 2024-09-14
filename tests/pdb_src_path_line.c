#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/plugins.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_source_path_line_memory.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Src] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Src]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Src]->win;
    win->file_data = (file_data_t*) malloc (sizeof (file_data_t));

    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };

    state->command = cmd;
    state->debugger->index = DEBUGGER_PDB;

    start_debugger (state);

    insert_output_start_marker (state);
    send_command (state, "where\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_source_path_line_memory (state);

    printf ("%s (%d)\n\n", win->file_data->path, win->file_data->line);

    return 0;
}


