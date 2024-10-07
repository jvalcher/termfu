#include <string.h>

#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"

static int get_program_output_gdb (state_t *state);
static int get_program_output_pdb (state_t *state);



int
get_program_output (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            ret = get_program_output_gdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get program output (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            ret = get_program_output_pdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get program output (PDB)");
            }
            break;
    }

    return A_OK;
}



static int
get_program_output_gdb (state_t *state)
{
    size_t       i;
    window_t    *win;
    char        *src_ptr;
    buff_data_t *dest_buff;

    char *new_run_str = "<New run>\n";

    win       = state->plugins[Prg]->win;
    src_ptr   = state->debugger->program_buffer;
    dest_buff = win->buff_data;

    if (dest_buff->new_data) {
        if (strstr (src_ptr, "error") == NULL) {

            // new run marker
            if (state->new_run) {
                for (i = 0; i < strlen (new_run_str); i++) {
                    cp_wchar (dest_buff, new_run_str [i]);
                }
                state->new_run = false;
            }

            while (*src_ptr != '\0') {
                cp_wchar (dest_buff, *src_ptr++);
            }
            dest_buff->changed = true;
        }
    }

    state->debugger->program_buffer [0] = '\0';
    dest_buff->new_data = false;

    return A_OK;
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
                cp_wchar (dest_buff, *src_ptr++);
            }
            dest_buff->changed = true;
        }
    }
    state->debugger->program_buffer [0] = '\0';
    dest_buff->new_data = false;

    return A_OK;
}

