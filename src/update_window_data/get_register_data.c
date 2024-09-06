#include <ctype.h>

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
    char *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Reg]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    // send debugger command
    insert_output_start_marker (state);
    send_command (state, "info registers\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    if (strstr (src_ptr, "error") == NULL) {

        dest_buff->buff_pos = 0;

        while (*src_ptr != '\0') {

            //  \\\t, \\\n
            if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                if (*(src_ptr + 1) == 'n') {
                    src_ptr += 2;
                } else if (*(src_ptr + 1) == 't') {
                    src_ptr += 2;
                } 
            }

            //  \\\\  ->  '\\'
            else if (*src_ptr == '\\' && *(src_ptr + 1) == '\\' ) {
                src_ptr += 1;
            }

            else {
                cp_char (dest_buff, *src_ptr++);
            }
        }

        dest_buff->changed = true;
    }
}





