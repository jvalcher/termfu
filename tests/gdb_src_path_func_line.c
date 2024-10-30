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
    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Src;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    buff_data_t *buff_data = win->buff_data;

    ////////////

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    putchar ('\n');

    // program not running
    get_source_path_line_memory (state);
    printf ("Source path: \"%s\"\n", debugger->src_path_buffer);
    printf ("Source line: %d\n", debugger->curr_Src_line);
    printf ("Function: \"%s\"\n", debugger->curr_func);

    putchar ('\n');

    // program running
    send_command_mp (state, "-break-insert hello2.c:9\n");
    send_command_mp (state, "-exec-run\n");
    get_source_path_line_memory (state);
    printf ("Source path: \"%s\"\n", debugger->src_path_buffer);
    printf ("Source line: %d\n", debugger->curr_Src_line);
    printf ("Function: \"%s\"\n", debugger->curr_func);

    return 0;
}
