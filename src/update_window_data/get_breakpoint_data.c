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
    int i;
    window_t *win;
    const char *key_number   = "number=\"",
               *key_orig_loc = "original-location=\"";
    char *src_ptr,
          break_buff [BREAK_LEN];
    buff_data_t *dest_buff;
    breakpoint_t *tmp_break,
                 *curr_break;

    win       = state->plugins[Brk]->win;
    src_ptr   = state->debugger->data_buffer;
    dest_buff = win->buff_data;

    // free breakpoints
    if (state->breakpoints != NULL) {
        curr_break = state->breakpoints;
        do {
            tmp_break = curr_break->next;
            free (curr_break);
            curr_break = tmp_break;  
        } while (curr_break != NULL);
    }
    state->breakpoints = NULL;

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
            i = 0;
            src_ptr = strstr (src_ptr, key_orig_loc);
            src_ptr += strlen (key_orig_loc);
            while (*src_ptr != '\"') {

                break_buff [i++] = *src_ptr;
                cp_char (dest_buff, *src_ptr++);
            }
            break_buff [i] = '\0';

            // allocate breakpoint
            if (state->breakpoints == NULL) {
                curr_break = (breakpoint_t*) malloc (sizeof (breakpoint_t));
                state->breakpoints = curr_break;
                strncpy (curr_break->path_line, break_buff, BREAK_LEN - 1);
                curr_break->next = NULL;
            } else {
                curr_break->next = (breakpoint_t*) malloc (sizeof (breakpoint_t));
                curr_break = curr_break->next;
                strncpy (curr_break->path_line, break_buff, BREAK_LEN - 1);
                curr_break->next = NULL;
            }

            cp_char (dest_buff, '\n');
        }

        dest_buff->changed = true;
    }
}




