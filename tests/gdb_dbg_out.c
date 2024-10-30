#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/utilities.h"
#include "../src/update_window_data/get_debugger_output.h"


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

    putchar ('\n');

    win->buff_data->new_data = true;
    send_command_mp (state, "-break-insert main\n");
    printf ("-break-insert CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    
    win->buff_data->new_data = true;
    send_command_mp (state, "-exec-run\n");
    printf ("-exec-run CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    
    win->buff_data->new_data = true;
    send_command_mp (state, "-exec-next\n");
    printf ("-exec-next CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    
    win->buff_data->new_data = true;
    send_command_mp (state, "-exec-continue\n");
    printf ("-exec-continue CLI: \n\n%s\n\n", state->debugger->cli_buffer);

    return 0;
}
