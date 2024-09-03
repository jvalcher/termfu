
#include "get_register_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"


static void get_register_data_gdb (state_t *state);



void
get_register_data (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_register_data_gdb (state);
            break;
    }
}


static void
get_register_data_gdb (state_t *state)
{
    window_t *win;
    char *src_ptr,
         *dest_ptr;

    win      = state->plugins[Reg]->win;
    src_ptr  = state->debugger->cli_buffer;
    dest_ptr = win->buff_data->buff;

    // send debugger command
    insert_output_start_marker (state);
    send_command (state, "info registers\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    if (strstr (src_ptr, "error") == NULL) {

        win->buff_data->buff_pos = 0;

        while (*src_ptr != '\0') {
            *dest_ptr++ = *src_ptr++;
        }
        *dest_ptr = '\0';

        win->buff_data->changed = true;
    }
}






