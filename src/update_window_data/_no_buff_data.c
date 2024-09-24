#include "../data.h"
#include "../utilities.h"



void
no_buff_data (int      plugin_index,
              state_t *state)
{
    buff_data_t *data = state->plugins[plugin_index]->win->buff_data;
    char *msg = concatenate_strings (2, NO_DATA_MSG, state->debugger->title),
         *ptr = msg;

    data->buff_pos = 0;
    while (*ptr != '\0') {
        cp_char (data, *ptr++);
    }

    free (msg);
}
