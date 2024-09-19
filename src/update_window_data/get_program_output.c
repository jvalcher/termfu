#include <string.h>

#include "get_program_output.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"
#include "_no_buff_data.h"

static void get_program_output_gdb (state_t *state);
static void get_program_output_pdb (state_t *state);



void
get_program_output (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_program_output_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_program_output_pdb (state);
            break;
    }
}



static void
get_program_output_gdb (state_t *state)
{
    window_t *win;
    char     *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Prg]->win;
    src_ptr   = state->debugger->program_buffer;
    dest_buff = win->buff_data;

    if (dest_buff->new_data) {
        if (strstr (src_ptr, "error") == NULL) {
            while (*src_ptr != '\0') {
                cp_char (dest_buff, *src_ptr++);
            }
            dest_buff->changed = true;
        }
    }
    state->debugger->program_buffer [0] = '\0';
    dest_buff->new_data = false;
}



static void
get_program_output_pdb (state_t *state)
{
    no_buff_data (Prg, state); 

    state->plugins[Prg]->win->buff_data->changed = true;
}

