#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/plugins.h"
#include "../src/utilities.h"
#include "../src/update_window_data/get_source_path_line_memory.h"


int
main (void)
{
    //////////// allocate structs
    //////////// set plugin_index variables

    //int plugin_index = Src;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    debugger_t *debugger   = state->debugger;
    //plugin_t *plugin       = state->plugins[plugin_index];
    //window_t *win          = plugin->win;
    //buff_data_t *buff_data = win->buff_data;

    ////////////

    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;
    debugger->index = DEBUGGER_PDB;

    start_debugger (state);

    get_source_path_line_memory (state);
    printf ("%s (%d)\n\n", debugger->src_path_buffer, debugger->curr_Src_line);

    send_command_mp (state, "break 10\n");
    send_command_mp (state, "continue\n");

    get_source_path_line_memory (state);
    printf ("%s (%d)\n\n", debugger->src_path_buffer, debugger->curr_Src_line);

    return 0;
}


