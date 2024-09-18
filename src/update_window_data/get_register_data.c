#include <ctype.h>

#include "get_register_data.h"
#include "_no_buff_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"


static void get_register_data_gdb (state_t *state);
static void get_register_data_pdb (state_t *state);



void
get_register_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_register_data_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_register_data_pdb (state);
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

    send_command_mp (state, "info registers\n");

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



static void
get_register_data_pdb (state_t *state)
{
    no_buff_data (Reg, state); 

    state->plugins[Reg]->win->buff_data->changed = true;
}

