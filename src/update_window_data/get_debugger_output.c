#include <string.h>

#include "get_debugger_output.h"
#include "../data.h"
#include "../plugins.h"

static void get_debugger_output_gdb (state_t *state);



void
get_debugger_output (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_debugger_output_gdb (state);
            break;
    }
}


static void
get_debugger_output_gdb (state_t *state)
{
    window_t *win;
    char     *src_ptr,
             *dest_ptr;

    win      = state->plugins[Dbg]->win;
    src_ptr  = state->debugger->cli_buffer;
    dest_ptr = win->buff_data->buff;
    dest_ptr += strlen (dest_ptr);

    // create buffer
    if (strstr (src_ptr, "error") == NULL) {

        while (*src_ptr != '\0') {
            *dest_ptr++ = *src_ptr++; 
        }
        *dest_ptr++ = '\n';
        *dest_ptr = '\0';

        win->buff_data->changed = true;
    }
}




