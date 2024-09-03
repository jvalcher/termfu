#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_register_data.h"
#include "../src/start_debugger.h"
#include "../src/parse_cli_arguments.h"
#include "../src/insert_output_marker.h"
#include "../src/parse_debugger_output.h"
#include "../src/utilities.h"



int
main (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Reg] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Reg]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Reg]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    state->plugins[Reg]->win->buff_data->buff = malloc (Reg_BUF_LEN);
    window_t *win = state->plugins[Reg]->win;
    win->has_data_buff = true;
    
    // start debugger
    parse_cli_arguments (argc, argv, state->debugger);
    start_debugger (state);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // insert breakpoint
    insert_output_start_marker (state);
    send_command (state, "-break-insert 12\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // run
    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_register_data (state);

    printf ("%s\n", win->buff_data->buff);

    return 0;
}

