#include <stdlib.h>
#include <string.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/update_window_data/get_local_vars.h"

//
// REQUIRES: comment out 'send_command_mp (state, "locals()\n");' in get_local_vars_pdb()
//

int
main (void)
{
    char *str1 = "{'num1': 154, 'num2': 98, 'narr': [1, 2, 3, 4, 5], 'sarr': ['one', 'two', 'three'], 'str1': 'Hello, world!'}";
    char *str2 = "{}";

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[LcV] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[LcV]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[LcV]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    window_t *win = state->plugins[LcV]->win;
    win->buff_data->buff = (char*) malloc (sizeof (char) * LcV_BUF_LEN);
    win->buff_data->buff_len = LcV_BUF_LEN;

    state->debugger->index = DEBUGGER_PDB;
    state->debugger->program_len = PROGRAM_BUF_LEN;

    strncpy (state->debugger->program_buffer, str1, PROGRAM_BUF_LEN - 1);
    get_local_vars (state);
    printf ("&&&\n%s\n&&&\n", win->buff_data->buff);

    strncpy (state->debugger->program_buffer, str2, PROGRAM_BUF_LEN - 1);
    get_local_vars (state);
    printf ("&&&\n%s\n&&&\n", win->buff_data->buff);

    return 0;
}
