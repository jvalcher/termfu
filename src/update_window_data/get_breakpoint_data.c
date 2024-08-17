#include "get_breakpoint_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"
#include "../display_lines.h"


static void get_breakpoint_data_gdb (state_t *state);



void
get_breakpoint_data (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_breakpoint_data_gdb (state);
            break;
    }
}


static void
get_breakpoint_data_gdb (state_t *state)
{
    window_t *win;
    const
    char *key_number   = "number=\"",
         *key_orig_loc = "original-location=\"";
    char *src_ptr,
         *dest_ptr;

    win      = state->plugins[Brk]->win;
    src_ptr  = state->debugger->data_buffer;
    dest_ptr = win->buff_data->buff;

    // send debugger command
    insert_output_start_marker (state);
    send_command (state, "-break-info\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // format data
    if (strstr (src_ptr, "error") == NULL) {

        // get breakpoint number
        while ((src_ptr = strstr (src_ptr, key_number)) != NULL) {
            src_ptr += strlen (key_number);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
            }

            *dest_ptr++ = ':';
            *dest_ptr++ = ' ';

            // get location
            src_ptr = strstr (src_ptr, key_orig_loc);
            src_ptr += strlen (key_orig_loc);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
            }

            *dest_ptr++ = '\n';
        }
        *dest_ptr = '\0';

        set_buff_rows_cols (win);
    }
}




