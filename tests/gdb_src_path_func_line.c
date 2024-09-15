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
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->debugger->index = DEBUGGER_GDB;

    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[Src] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Src]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Src]->win;

    win->file_data = (file_data_t*) malloc (sizeof (file_data_t));
    win->file_data->path_changed = true;
    win->file_data->path_len = FILE_PATH_LEN;
    win->file_data->addr_len = ADDRESS_LEN;
    win->file_data->func_len = FUNC_LEN;

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    putchar ('\n');

    // program not running
    get_source_path_line_memory (state);

    printf ("%s\n", win->file_data->path);
    printf ("%s\n", win->file_data->func);
    printf ("%d\n", win->file_data->line);

    putchar ('\n');

    // program running
    insert_output_start_marker (state);
    send_command (state, "-break-insert hello2.c:9\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_source_path_line_memory (state);

    printf ("%s\n", win->file_data->path);
    printf ("%s\n", win->file_data->func);
    printf ("%d\n", win->file_data->line);

    return 0;
}
