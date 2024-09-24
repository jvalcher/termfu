#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"
#include "../update_window_data/_update_window_data.h"


// TODO: save commands, up/down key access

int
run_custom_command (state_t *state)
{
    char *cmd;

    if (get_popup_window_input  (">> ", state->input_buffer) == RET_FAIL) {
        pfemr ("Failed to get custom command input");
    }

    cmd = concatenate_strings (2, state->input_buffer, "\n");    
    if (send_command_mp (state, cmd) == RET_FAIL) {
        pfemr ("Failed to send custom command");
    }
    free (cmd);
    
    state->plugins[Dbg]->win->buff_data->new_data = true;
    state->plugins[Prg]->win->buff_data->new_data = true;
    if (update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat) == RET_FAIL) {
        pfemr ("Failed to update windows");
    }

    return RET_OK;
}



