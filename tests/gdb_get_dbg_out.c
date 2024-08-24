#include "test_utilities.h"

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/parse_cli_arguments.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_debugger_output.h"


int
main (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Dbg] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Dbg]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Dbg]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    window_t *win = state->plugins[Dbg]->win;

    // start debugger
    parse_cli_arguments (argc, argv, state->debugger);
    start_debugger (state);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // get debugger output
    insert_output_start_marker (state);
    send_command (state, "-break-insert main\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_debugger_output (state);
    printf ("%s", win->buff_data->buff);
    
    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_debugger_output (state);
    printf ("%s", win->buff_data->buff);
    
    insert_output_start_marker (state);
    send_command (state, "-exec-next\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_debugger_output (state);
    printf ("%s", win->buff_data->buff);
    
    insert_output_start_marker (state);
    send_command (state, "-exec-continue\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_debugger_output (state);
    printf ("%s", win->buff_data->buff);

    return 0;
}
