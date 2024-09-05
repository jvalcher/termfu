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
    char *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[LcV]->win;
    src_ptr   = state->debugger->data_buffer;
    dest_buff = win->buff_data;

    // send debugger command
    insert_output_start_marker (state);
    send_command (state, "-stack-list-locals 1\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    if (strstr (src_ptr, "error") == NULL) {

        dest_buff->buff_pos = 0;

        while ((src_ptr = strstr (src_ptr, key_name)) != NULL) {
            src_ptr += strlen (key_name);
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, ' ');
            cp_char (dest_buff, '=');
            cp_char (dest_buff, ' ');

            // get path:line
            src_ptr = strstr (src_ptr, key_value);
            src_ptr += strlen (key_value);
            while (*src_ptr != '}') {

                //  \\\t, \\\n
                if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                    if (*(src_ptr + 1) == 'n') {
                        cp_char (dest_buff, '\\');
                        cp_char (dest_buff, 'n');
                        src_ptr += 2;
                    } else if (*(src_ptr + 1) == 't') {
                        cp_char (dest_buff, '\\');
                        cp_char (dest_buff, 't');
                        src_ptr += 2;
                    } 
                }

                //  \\\"  ->  \"
                else if (*src_ptr == '\\' && *(src_ptr + 1) == '\"' ) {
                    src_ptr += 1;
                    cp_char (dest_buff, *src_ptr++);
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
                    cp_char (dest_buff, *src_ptr++);
                }
            }

            cp_char (dest_buff, '\n');
        }

        dest_buff->changed = true;
    }
}





