#include <string.h>
#include <ctype.h>

#include "get_debugger_output.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"

static void get_debugger_output_gdb (state_t *state);
static void get_debugger_output_pdb (state_t *state);



void
get_debugger_output (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_debugger_output_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_debugger_output_pdb (state);
            break;
    }
}


static void
get_debugger_output_gdb (state_t *state)
{
    window_t *win;
    char     *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Dbg]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    if (dest_buff->new_data) {
        if (strstr (src_ptr, "error") == NULL) {
            while (*src_ptr != '\0') {

                //  \\\t, \\\n
                if (*src_ptr == '\\' && isalpha(*(src_ptr + 1))) {
                    if (*(src_ptr + 1) == 'n') {
                        src_ptr += 2;
                    } else if (*(src_ptr + 1) == 't') {
                        src_ptr += 2;
                    } 
                }

                cp_char (dest_buff, *src_ptr++);
            }
            dest_buff->changed = true;
        }
    }
    dest_buff->new_data = false;
}



static void
get_debugger_output_pdb (state_t *state)
{
    window_t *win;
    char     *src_ptr;
    buff_data_t *dest_buff;

    win       = state->plugins[Dbg]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    if (dest_buff->new_data) {

        while (*src_ptr != '\0') {

            if (*src_ptr == '\'') {
                ++src_ptr;
            } 

            else {
                cp_char (dest_buff, *src_ptr++);
            }
        }

        dest_buff->changed = true;
    }

    dest_buff->new_data = false;
}

