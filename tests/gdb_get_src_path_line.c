#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/parse_cli_arguments.h"
#include "../src/update_window_data/get_source_file_and_line_number.h"
#include "../src/plugins.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"

int
main (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Src] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Src]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Src]->win->file_data = (file_data_t*) malloc (sizeof (file_data_t));
    window_t *win = state->plugins[Src]->win;

    // start debugger
    parse_cli_arguments (argc, argv, state->debugger);
    start_debugger (state);

    // program not running
    get_source_file_path_and_line_number (state);
    printf ("%s\n", win->file_data->path);
    printf ("%d\n", win->file_data->line);

    // program running
    insert_output_start_marker (state);
    send_command (state, "-break-insert hello_2.c:8\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);
        //
    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);
        // 
    get_source_file_path_and_line_number (state);
    printf ("%s\n", win->file_data->path);
    printf ("%d\n", win->file_data->line);

    return 0;
}
