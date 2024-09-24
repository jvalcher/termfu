#include <string.h>

#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"

static int get_program_output_gdb (state_t *state);
static int get_program_output_pdb (state_t *state);



int
get_program_output (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_program_output_gdb (state) == RET_FAIL) {
                pfemr ("Failed to get program output (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            if (get_program_output_pdb (state) == RET_FAIL) {
                pfemr ("Failed to get program output (PDB)");
            }
            break;
    }

    return RET_OK;
}



static int
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

    return RET_OK;
}



static int
get_program_output_pdb (state_t *state)
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

    return RET_OK;
}

