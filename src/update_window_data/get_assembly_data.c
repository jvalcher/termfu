#include <string.h>
#include <ctype.h>

#include "get_assembly_data.h"
#include "_no_buff_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"

#define OFFSET_COLS 4

static int get_assembly_data_gdb (state_t *state);
static int get_assembly_data_pdb (state_t *state);



int
get_assembly_data (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            ret = get_assembly_data_gdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get assembly data (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            ret = get_assembly_data_pdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get assembly data (PDB)");
            }
            break;
    }

    return A_OK;
}


static int
get_assembly_data_gdb (state_t *state)
{
    int          ret;
    window_t    *win;
    char        *src_ptr,
                *dest_ptr,
                *data_ptr,
                *cmd,
                *func;
    buff_data_t *dest_data;

    win       = state->plugins[Asm]->win;
    src_ptr   = state->debugger->cli_buffer;
    data_ptr  = state->debugger->data_buffer;
    dest_data = win->buff_data;

    func = (state->debugger->curr_func[0] == '\0') ? "main" : state->debugger->curr_func;
    cmd = concatenate_strings (3, "disassemble ", func, "\n");
    ret = send_command_mp (state, cmd);
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }

    free (cmd);

    if (strstr (data_ptr, "error") == NULL) {

        dest_data->buff_pos = 0;

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
                cp_wchar (dest_data, *src_ptr++);
            }
        }
        state->debugger->cli_buffer[0]  = '\0';
        state->debugger->data_buffer[0] = '\0';

        dest_data->changed = true;
    }

    // set scroll row on '=>'
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

    return A_OK;
}



static int
get_assembly_data_pdb (state_t *state)
{
    no_buff_data (Asm, state); 

    state->plugins[Asm]->win->buff_data->changed = true;

    return A_OK;
}

