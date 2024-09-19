#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/utilities.h"
#include "../src/update_window_data/get_debugger_output.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[Dbg] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Dbg]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Dbg]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Dbg_BUF_LEN);
    win->buff_data->buff_pos = 0;
    win->buff_data->buff_len = Dbg_BUF_LEN;
    win->buff_data->new_data = true;
    state->debugger->index = DEBUGGER_GDB;

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    putchar ('\n');

    win->buff_data->new_data = true;
    send_command_mp (state, "-break-insert main\n");
    printf ("-break-insert CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    get_debugger_output (state);
    
    win->buff_data->new_data = true;
    send_command_mp (state, "-exec-run\n");
    printf ("-exec-run CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    get_debugger_output (state);
    
    win->buff_data->new_data = true;
    send_command_mp (state, "-exec-next\n");
    printf ("-exec-next CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    get_debugger_output (state);
    
    win->buff_data->new_data = true;
    send_command_mp (state, "-exec-continue\n");
    printf ("-exec-continue CLI: \n\n%s\n\n", state->debugger->cli_buffer);
    get_debugger_output (state);

    printf ("DBG OUT: \n\n%s\n\n", win->buff_data->buff);

    return 0;
}
