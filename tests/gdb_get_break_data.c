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
    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[Brk] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Brk]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Brk]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Brk_BUF_LEN);
    win->buff_data->buff_pos = 0;
    win->buff_data->buff_len = Brk_BUF_LEN;
    state->debugger->index = DEBUGGER_GDB;
    win->buff_data->new_data = true;

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    // create breakpoints for ../misc/hello
    insert_output_start_marker (state);
    send_command (state, "-break-insert 16\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "-break-insert hello.c:17\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "-break-insert hello2.c:9\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // get data
    get_breakpoint_data (state);

    printf ("\nWIN (%lu): \n\n%s\n", strlen(win->buff_data->buff), win->buff_data->buff);

    return 0;
}
