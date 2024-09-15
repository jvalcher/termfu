#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_register_data.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/parse_debugger_output.h"
#include "../src/utilities.h"



int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[Reg] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Reg]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Reg]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Reg_BUF_LEN);
    win->buff_data->buff_pos = 0;
    win->buff_data->buff_len = Reg_BUF_LEN;
    win->buff_data->new_data = true;
    state->debugger->index = DEBUGGER_GDB;

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    putchar ('\n');

    win->buff_data->new_data = true;
    insert_output_start_marker (state);
    send_command (state, "-break-insert 12\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    win->buff_data->new_data = true;
    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    get_register_data (state);

    printf ("%s\n", win->buff_data->buff);

    return 0;
}

