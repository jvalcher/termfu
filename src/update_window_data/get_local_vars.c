#include <ctype.h>

#include "get_local_vars.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"


static void get_local_vars_gdb (state_t *state);



void
get_local_vars (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_local_vars_gdb (state);
            break;
    }
}


static void
get_local_vars_gdb (state_t *state)
{
    window_t *win;
    const
    char *key_name  = "name=\"",
         *key_value = "value=\"";
    char *src_ptr,
         *dest_ptr;

    win      = state->plugins[LcV]->win;
    src_ptr  = state->debugger->data_buffer;
    dest_ptr = win->buff_data->buff;

    // send debugger command
    insert_output_start_marker (state);
    send_command (state, "-stack-list-locals 1\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    if (strstr (src_ptr, "error") == NULL) {

        win->buff_data->buff_pos = 0;

        while ((src_ptr = strstr (src_ptr, key_name)) != NULL) {
            src_ptr += strlen (key_name);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
            }

            *dest_ptr++ = ' ';
            *dest_ptr++ = '=';
            *dest_ptr++ = ' ';

            // get path:line
            src_ptr = strstr (src_ptr, key_value);
            src_ptr += strlen (key_value);
            while (*src_ptr != '}') {

                //  \\\t, \\\n
                if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                    if (*(src_ptr + 1) == 'n') {
                        *dest_ptr++ = '\\';
                        *dest_ptr++ = 'n';
                        src_ptr += 2;
                    } else if (*(src_ptr + 1) == 't') {
                        *dest_ptr++ = '\\';
                        *dest_ptr++ = 't';
                        src_ptr += 2;
                    } 
                }

                //  \\\"  ->  \"
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\"' ) {
                    src_ptr += 1;
                    *dest_ptr++  = *src_ptr++;
                }

                //  \\\\  ->  skip
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\\' ) {
                    src_ptr += 1;
                }

                //  \"  ->  skip
                else if (*src_ptr == '\"') {
                    src_ptr += 1;
                }

                else {
                    *dest_ptr++  = *src_ptr++;
                }
            }
            *dest_ptr++ = '\n';
        }
        *dest_ptr = '\0';

        win->buff_data->changed = true;
    }
}





