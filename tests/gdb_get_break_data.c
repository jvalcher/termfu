/*
    ./build_run get_break_data.c ../misc/hello
*/

#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_breakpoint_data.h"
#include "../src/plugins.h"


int
main (void)
{
    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Brk;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    //debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    buff_data_t *buff_data = win->buff_data;

    ////////////

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    send_command_mp (state, "-break-insert 16\n");
    send_command_mp (state, "-break-insert hello.c:17\n");
    send_command_mp (state, "-break-insert hello2.c:9\n");

    get_breakpoint_data (state);
    printf ("\nWIN (%lu): \n\n%s\n", strlen(buff_data->buff), buff_data->buff);

    return 0;
}
