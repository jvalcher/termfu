#include <ctype.h>

#include "get_watchpoint_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"


static void get_watchpoint_data_gdb (state_t *state);



void
get_watchpoint_data (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_watchpoint_data_gdb (state);
            break;
    }
}


static void
get_watchpoint_data_gdb (state_t *state)
{
    window_t *win;
    watchpoint_t *watch;
    const char *hex = "0x";
    char *cmd, *ptr,
         *src_ptr,
         *dest_ptr,
         *data_ptr,
         *watch_val;

    win      = state->plugins[Wat]->win;
    dest_ptr = win->buff_data->buff;
    watch    = win->buff_data->watchpoints;

    // create buffer, add values
    while (watch != NULL) {

        // send watchpoint command
        cmd = concatenate_strings (3, "print ", watch->var, "\n");    
        insert_output_start_marker (state);
        send_command (state, cmd);
        insert_output_end_marker (state);
        parse_debugger_output (state);
        free (cmd);

        src_ptr  = state->debugger->cli_buffer;
        data_ptr = state->debugger->data_buffer;
        watch_val = watch->value;

        // index
        *dest_ptr++ = '(';
        dest_ptr += sprintf (dest_ptr, "%d", watch->index);
        *dest_ptr++ = ')';
        *dest_ptr++ = ' ';

        // variable
        dest_ptr += sprintf (dest_ptr, "%s = ", watch->var);

        if (strstr (data_ptr, "error") == NULL) {

            // value
                // skip hex value or '='
            ptr = src_ptr;
            src_ptr = strstr (src_ptr, hex);
            if (src_ptr != NULL) {
                src_ptr += strlen (hex);
            } else {
                src_ptr = ptr;
                src_ptr = strstr (src_ptr, "=");
                src_ptr += 1;
            }

            src_ptr = strstr (src_ptr, " ");

            while (*src_ptr != '\n') {

                // ==  \\\t,\n, etc. -> skip
                if (*src_ptr == '\\' && (isalpha(*(src_ptr + 1)) || *(src_ptr + 1) == '\n') ) {
                    src_ptr += 2;
                }

                // ==  \\\"  ->  \"
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\"' ) {
                    src_ptr += 1;
                    *dest_ptr++  = *src_ptr;
                    *watch_val++ = *src_ptr++;
                }

                /*
                // ==  \"  ->  skip
                else if (*src_ptr == '\"') {
                    src_ptr += 1;
                }
                */

                else {
                    *dest_ptr++  = *src_ptr;
                    *watch_val++ = *src_ptr++;
                }
            }
            *dest_ptr++ = '\n';
        }

        else {
            strcpy (watch->value, "none");
            strcpy (dest_ptr, "none\n");
            dest_ptr += 5;
        }

        *watch_val = '\0';

        watch = watch->next;
    }
    *dest_ptr = '\0';

    win->buff_data->changed = true;
}





