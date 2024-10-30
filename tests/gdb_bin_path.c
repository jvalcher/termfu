#include <stdlib.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/update_window_data/get_binary_path_time.h"
#include "../src/utilities.h"


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

    //debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    //buff_data_t *buff_data = win->buff_data;

    ////////////

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    get_binary_path_time (state);

    printf ("\n%s\n", state->debugger->prog_path);

    return 0;
}
