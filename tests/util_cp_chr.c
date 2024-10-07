#include <stdlib.h>
#include <string.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/utilities.h"



int
main (void)
{
    int i;
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->debugger->data_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN);
    state->debugger->data_len = ORIG_BUF_LEN;
    state->debugger->data_pos = 0;
    state->debugger->data_times_doubled = 0;
    debugger_t *dbgr = state->debugger;
    set_num_plugins (state);
    state->plugins = (plugin_t**) malloc (sizeof (char) * state->num_plugins);
    state->plugins[Dbg] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Dbg]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Dbg]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * ORIG_BUF_LEN); 
    win->buff_data->buff_len = ORIG_BUF_LEN;
    win->buff_data->buff_pos = 0;

    // calculate maximum buffer size before loop around
    int max_size = ORIG_BUF_LEN;
    for (i = 0; i < MAX_DOUBLE_TIMES; i++) {
        max_size *= 2;
    }
    printf ("Max buffer size: %d\n", max_size);

    printf ("\nPress any key to continue...\n\n");
    getchar();

    max_size += 8;

    // debugger_t buffer
    for (i = 0; i < max_size; i++) {
        cp_dchar (dbgr, 'a', DATA_BUF);
        printf ("pos: %d,  len: %d,  doubled: %d,  strlen: %ld\n",
                    dbgr->data_pos,
                    dbgr->data_len,
                    dbgr->data_times_doubled,
                    strlen (dbgr->data_buffer));
    }

    printf ("\nPress any key to continue...\n\n");
    getchar();

    // buff_data_t buffer
    for (i = 0; i < max_size; i++) {
        cp_wchar (win->buff_data, 'a');
        printf ("pos: %d,  len: %d,  doubled: %d,  strlen: %ld\n",
                    win->buff_data->buff_pos,
                    win->buff_data->buff_len,
                    win->buff_data->times_doubled,
                    strlen (win->buff_data->buff));
    }

    return 0;
}
