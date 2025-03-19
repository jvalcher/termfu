#include <string.h>
#include <ctype.h>

#include "get_assembly_data.h"
#include "_no_buff_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../plugins.h"

#define OFFSET_COLS 4

static int get_assembly_data_gdb (state_t *state);
static int get_assembly_data_pdb (state_t *state);



int
get_assembly_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_assembly_data_gdb (state) == FAIL)
                pfemr ("Failed to get assembly data (GDB)");
            break;
        case (DEBUGGER_PDB):
            if (get_assembly_data_pdb (state) == FAIL)
                pfemr ("Failed to get assembly data (PDB)");
            break;
    }
    return A_OK;
}


static int
get_assembly_data_gdb (state_t *state)
{
    window_t    *win;
    char        *src_ptr,
                *dest_ptr,
                *data_ptr,
                *func;
    buff_data_t *dest_data;

    win       = state->plugins[Asm]->win;
    src_ptr   = state->debugger->cli_buffer;
    data_ptr  = state->debugger->data_buffer;
    dest_data = win->buff_data;

    func = (state->debugger->curr_func[0] == '\0') ? "main" : state->debugger->curr_func;
    if (send_command_mp (state, "disassemble ", func, "\n") == FAIL)
        pfemr (ERR_DBG_CMD);

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

        dest_data->changed = true;
    }

    // set state->debugger->curr_Asm_line to '=>'
    dest_ptr = win->buff_data->buff;
    state->debugger->curr_Asm_line = 1;
    while (*dest_ptr != '\0') {
        if ( *dest_ptr      == '=' &&
            *(dest_ptr + 1) == '>') {
            break;
        }
        if (*dest_ptr == '\n') {
            ++state->debugger->curr_Asm_line;
        }
        ++dest_ptr;
    }

    return A_OK;
}



static int
get_assembly_data_pdb (state_t *state)
{
    no_buff_data (Asm, state); 

    state->debugger->curr_Asm_line = 1;
    state->plugins[Asm]->win->buff_data->scroll_col_offset = 0;
    state->plugins[Asm]->win->buff_data->changed = true;

    return A_OK;
}

