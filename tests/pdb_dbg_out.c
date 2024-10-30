#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_debugger_output.h"


int
main (void)
{
    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Dbg;

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

    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;
    debugger->index = DEBUGGER_PDB;
    start_debugger (state);

    send_command_mp (state, "break 10\n");
    get_debugger_output (state);
    printf ("break 10: \n%s\n\n", win->buff_data->buff);
    
    send_command_mp (state, "restart\n");
    get_debugger_output (state);
    printf ("restart: \n%s\n\n", win->buff_data->buff);
    
    send_command_mp (state, "next\n");
    get_debugger_output (state);
    printf ("next: \n%s\n\n", win->buff_data->buff);
    
    send_command_mp (state, "next\n");
    get_debugger_output (state);
    printf ("next: \n%s\n\n", win->buff_data->buff);
    
    send_command_mp (state, "continue\n");
    get_debugger_output (state);
    printf ("continue: \n%s\n\n", win->buff_data->buff);

    return 0;
}

