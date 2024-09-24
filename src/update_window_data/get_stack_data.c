#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"


static int get_stack_data_gdb (state_t *state);
static int get_stack_data_pdb (state_t *state);



int
get_stack_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_stack_data_gdb (state) == RET_FAIL) {
                pfemr ("Failed to get stack data (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            if (get_stack_data_pdb (state) == RET_FAIL) {
                pfemr ("Failed to get stack data (PDB)");
            }
            break;
    }

    return RET_OK;
}


static int
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

    if (send_command_mp (state, "-stack-list-frames\n") == RET_FAIL) {
        pfemr ("Failed to send stack data command (GDB)");
    }

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

    return RET_OK;
}



static int
get_stack_data_pdb (state_t *state)
{
    window_t *win;
    char     *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Stk]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    if (send_command_mp (state, "where\n") == RET_FAIL) {
        pfemr ("Failed to send stack data command (PDB)");
    }

    dest_buff->buff_pos = 0;

    while (*src_ptr != '\0') {
        cp_char (dest_buff, *src_ptr++);
    }

    dest_buff->changed = true;
    state->debugger->cli_buffer[0] = '\0';
    dest_buff->new_data = false;

    return RET_OK;
}

