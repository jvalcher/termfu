#include <ctype.h>

#include "_no_buff_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"


static int get_register_data_gdb (state_t *state);
static int get_register_data_pdb (state_t *state);



int
get_register_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_register_data_gdb (state) == RET_FAIL) {
                pfemr ("Failed to get register data (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            if (get_register_data_pdb (state) == RET_FAIL) {
                pfemr ("Failed to get register data (PDB)");
            }
            break;
    }

    return RET_OK;
}


static int
get_register_data_gdb (state_t *state)
{
    window_t *win;
    char *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Reg]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    if (send_command_mp (state, "info registers\n") == RET_FAIL) {
        pfemr ("Failed to send register data command (GDB)");
    }

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

    state->debugger->cli_buffer[0] = '\0';

    return RET_OK;
}



static int
get_register_data_pdb (state_t *state)
{
    no_buff_data (Reg, state); 

    state->plugins[Reg]->win->buff_data->changed = true;

    return RET_OK;
}

