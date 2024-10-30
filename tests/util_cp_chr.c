#include <stdlib.h>
#include <string.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/utilities.h"
#include "../src/start_debugger.h"



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

    debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    //buff_data_t *buff_data = win->buff_data;

    ////////////

    int i;

    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;
    debugger->index = DEBUGGER_PDB;
    start_debugger (state);

    // calculate maximum buffer size before loop around
    int max_size = ORIG_BUF_LEN;
    for (i = 0; i < MAX_DOUBLE_TIMES; i++) {
        max_size *= 2;
    }
    printf ("Max buffer size: %d\n", max_size);
    max_size += 8;

    printf ("\nPress any key to continue...\n\n");
    getchar();

    // debugger_t buffer
    for (i = 0; i < max_size; i++) {
        cp_dchar (debugger, 'a', DATA_BUF);
        printf ("pos: %d,  len: %d,  doubled: %d,  strlen: %ld\n",
                    debugger->data_pos,
                    debugger->data_len,
                    debugger->data_times_doubled,
                    strlen (debugger->data_buffer));
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

    printf ("\n\n");

    return 0;
}
