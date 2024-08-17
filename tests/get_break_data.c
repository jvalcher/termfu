/*
    ./build_run get_break_data.c ../misc/hello
*/

#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/parse_cli_arguments.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/breakpoints.h"
#include "../src/plugins.h"


int
main (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Brk] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Brk]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Brk]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));

    // start debugger
    parse_cli_arguments (argc, argv, state->debugger);
    start_debugger (state);

    // create breakpoints for ../misc/hello
    insert_output_start_marker (state);
    send_command (state, "-break-insert 16\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);
    insert_output_start_marker (state);
    send_command (state, "-break-insert 17\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);
    insert_output_start_marker (state);
    send_command (state, "-break-insert 18\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // get data
    get_breakpoint_data (state);

    /*
    printf ("DATA (%lu): \n%s\n\n", strlen(state->debugger->data_buffer), state->debugger->data_buffer);
    printf ("CLI (%lu): \n%s\n\n", strlen(state->debugger->cli_buffer), state->debugger->cli_buffer);
    printf ("PROGRAM (%lu): \n%s\n\n", strlen(state->debugger->program_buffer), state->debugger->program_buffer);
    */
    printf ("\nWIN (%lu): \n\n%s\n", strlen(state->plugins[Brk]->win->buff_data->buff), state->plugins[Brk]->win->buff_data->buff);

    return 0;
}
