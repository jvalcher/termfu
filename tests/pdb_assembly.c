#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/update_window_data/_no_buff_data.h"


int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Asm] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Asm]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Asm]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Asm_BUF_LEN);
    win->buff_data->buff_len = Asm_BUF_LEN;

    no_buff_data (win->buff_data);

    printf ("%s\n", win->buff_data->buff);

    return 0;
}


