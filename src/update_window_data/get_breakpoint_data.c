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
    char *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Brk]->win;
    src_ptr   = state->debugger->data_buffer;
    dest_buff = win->buff_data;

    // send debugger command
    insert_output_start_marker (state);
    send_command (state, "-break-info\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // create buffer
    if (strstr (src_ptr, "error") == NULL) {

        dest_buff->buff_pos = 0;

        // get breakpoint number
        while ((src_ptr = strstr (src_ptr, key_number)) != NULL) {
            src_ptr += strlen (key_number);
            cp_char (dest_buff, '(');
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, ')');
            cp_char (dest_buff, ' ');

            // get file:line
            src_ptr = strstr (src_ptr, key_orig_loc);
            src_ptr += strlen (key_orig_loc);
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, '\n');
        }

        dest_buff->changed = true;
    }
}




