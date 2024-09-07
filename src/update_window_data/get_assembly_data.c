#include <string.h>
#include <ctype.h>

#include "get_assembly_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"

#define OFFSET_COLS 4


static void get_assembly_data_gdb (state_t *state);



void
get_assembly_data (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_assembly_data_gdb (state);
            break;
    }
}


static void
get_assembly_data_gdb (state_t *state)
{
    window_t *win;
    char *src_ptr,
         *dest_ptr,
         *data_ptr,
         *cmd,
         *func,
         *main = "main";
    buff_data_t *dest_buff;
    file_data_t *src_data;

    win       = state->plugins[Asm]->win;
    src_ptr   = state->debugger->cli_buffer;
    data_ptr  = state->debugger->data_buffer;
    dest_buff = win->buff_data;
    src_data  = state->plugins[Src]->win->file_data;

    // send debugger command
    func = (src_data->func[0] == '\0') ? main : src_data->func;
    cmd = concatenate_strings (3, "disassemble ", func, " \n");
        //
    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);
        //
    free (cmd);

    if (strstr (data_ptr, "error") == NULL) {

        dest_buff->buff_pos = 0;

        while (*src_ptr != '\0') {

            if ( *src_ptr      == 'D' &&
                *(src_ptr + 1) == 'u' &&
                *(src_ptr + 2) == 'm' &&
                *(src_ptr + 3) == 'p' &&
                *(src_ptr + 4) == ' ' &&
                *(src_ptr + 5) == 'o' &&
                *(src_ptr + 6) == 'f') {

                while (*src_ptr++ != '\n') {
                    ;
                }
            }

            if ( *src_ptr      == 'E' &&
                *(src_ptr + 1) == 'n' &&
                *(src_ptr + 2) == 'd' &&
                *(src_ptr + 3) == ' ' &&
                *(src_ptr + 4) == 'o' &&
                *(src_ptr + 5) == 'f') {

                while (*src_ptr++ != '\n') {
                    ;
                }
            }

            //  \\\t, \\\n
            else if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                if (*(src_ptr + 1) == 'n') {
                    src_ptr += 2;
                } else if (*(src_ptr + 1) == 't') {
                    src_ptr += 2;
                } 
            }

            else {
                cp_char (dest_buff, *src_ptr++);
            }
        }
        
        dest_buff->changed = true;
    }

    // set current line ('=>')
    dest_ptr = win->buff_data->buff;
    win->buff_data->scroll_row = 1;
    while (*dest_ptr != '\0') {
        if ( *dest_ptr      == '=' &&
            *(dest_ptr + 1) == '>') {
            break;
        }
        if (*dest_ptr == '\n') {
            ++win->buff_data->scroll_row;
        }
        ++dest_ptr;
    }
}




