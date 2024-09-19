#include <ctype.h>

#include "get_register_data.h"
#include "_no_buff_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"


static void get_stack_data_gdb (state_t *state);
static void get_stack_data_pdb (state_t *state);



void
get_stack_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_stack_data_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_stack_data_pdb (state);
            break;
    }
}


static void
get_stack_data_gdb (state_t *state)
{
    window_t *win;
    char     *src_ptr,
             *tmp_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Stk]->win;
    src_ptr   = state->debugger->data_buffer;
    dest_buff = win->buff_data;

    const char  *level_key = "level=\"",
                *addr_key  = "addr=\"",
                *func_key  = "func=\"",
                *file_key  = "file=\"",
                *line_key  = "line=\"";

    send_command_mp (state, "-stack-list-frames\n");

    dest_buff->buff_pos = 0;
    dest_buff->changed = true;

    if (strstr (src_ptr, "error") == NULL) {

        // level
        while ((src_ptr = strstr (src_ptr, level_key)) != NULL) {
            src_ptr += strlen (level_key);
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, ':');

            // addr
            src_ptr  = strstr (src_ptr, addr_key);
            src_ptr += strlen (level_key);
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, ' ');
            cp_char (dest_buff, ' ');

            // func
            src_ptr  = strstr (src_ptr, func_key);
            src_ptr += strlen (func_key);
            while (*src_ptr != '\"') {
                cp_char (dest_buff, *src_ptr++);
            }

            cp_char (dest_buff, ' ');
            cp_char (dest_buff, '-');
            cp_char (dest_buff, '-');
            cp_char (dest_buff, ' ');

            // file
            tmp_ptr = src_ptr;
            src_ptr  = strstr (src_ptr, file_key);
            if (src_ptr != NULL) {
                src_ptr += strlen (file_key);
                while (*src_ptr != '\"') {
                    cp_char (dest_buff, *src_ptr++);
                }

                cp_char (dest_buff, ':');

                // line
                src_ptr  = strstr (src_ptr, line_key);
                src_ptr += strlen (line_key);
                while (*src_ptr != '\"') {
                    cp_char (dest_buff, *src_ptr++);
                }
            } else {
                src_ptr = tmp_ptr;
            }

            cp_char (dest_buff, '\n');
        }
    }

    else {
        cp_char (dest_buff, '\0');
    }

    state->debugger->data_buffer[0] = '\0';
}



static void
get_stack_data_pdb (state_t *state)
{
    no_buff_data (Stk, state); 

    state->plugins[Stk]->win->buff_data->changed = true;
}

