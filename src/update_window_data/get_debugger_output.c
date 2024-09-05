#include <string.h>

#include "get_debugger_output.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"

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
    char     *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Dbg]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    // create buffer
    if (strstr (src_ptr, "error") == NULL) {

        while (*src_ptr != '\0') {
            cp_char (dest_buff, *src_ptr++);
        }

        dest_buff->changed = true;
    }
}




