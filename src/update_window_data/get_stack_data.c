#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../plugins.h"

static int get_stack_data_gdb (state_t *state);
static int get_stack_data_pdb (state_t *state);



int
get_stack_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_stack_data_gdb (state) == FAIL)
                pfemr ("Failed to get stack data (GDB)");
            break;
        case (DEBUGGER_PDB):
            if (get_stack_data_pdb (state) == FAIL)
                pfemr ("Failed to get stack data (PDB)");
            break;
    }
    return A_OK;
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

    dest_buff->buff_pos = 0;
    dest_buff->buff[0] = '\0';
    dest_buff->changed = true;

    if (send_command_mp (state, "-stack-list-frames\n") == FAIL)
        pfemr (ERR_DBG_CMD);

    if (strstr (src_ptr, "error") == NULL) {

        // level
        while ((src_ptr = strstr (src_ptr, level_key)) != NULL) {
            src_ptr += strlen (level_key);
            while (*src_ptr != '\"') {
                cp_wchar (dest_buff, *src_ptr++);
            }

            cp_wchar (dest_buff, ':');

            // addr
            src_ptr  = strstr (src_ptr, addr_key);
            src_ptr += strlen (level_key);
            while (*src_ptr != '\"') {
                cp_wchar (dest_buff, *src_ptr++);
            }

            cp_wchar (dest_buff, ' ');
            cp_wchar (dest_buff, ' ');

            // func
            src_ptr  = strstr (src_ptr, func_key);
            src_ptr += strlen (func_key);
            while (*src_ptr != '\"') {
                cp_wchar (dest_buff, *src_ptr++);
            }

            cp_wchar (dest_buff, ' ');
            cp_wchar (dest_buff, '-');
            cp_wchar (dest_buff, '-');
            cp_wchar (dest_buff, ' ');

            // file
            tmp_ptr = src_ptr;
            src_ptr  = strstr (src_ptr, file_key);
            if (src_ptr != NULL) {
                src_ptr += strlen (file_key);
                while (*src_ptr != '\"') {
                    cp_wchar (dest_buff, *src_ptr++);
                }

                cp_wchar (dest_buff, ':');

                // line
                src_ptr  = strstr (src_ptr, line_key);
                src_ptr += strlen (line_key);
                while (*src_ptr != '\"') {
                    cp_wchar (dest_buff, *src_ptr++);
                }
            } else {
                src_ptr = tmp_ptr;
            }

            cp_wchar (dest_buff, '\n');
        }
    }

    return A_OK;
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

    if (send_command_mp (state, "where\n") == FAIL)
        pfemr (ERR_DBG_CMD);

    dest_buff->buff_pos = 0;

    while (*src_ptr != '\0') {
        cp_wchar (dest_buff, *src_ptr++);
    }

    dest_buff->changed = true;
    state->debugger->cli_buffer[0] = '\0';
    dest_buff->new_data = false;

    return A_OK;
}

