#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"
#include "../update_window_data/_update_window_data.h"


// TODO: save commands, up/down key access

int
run_custom_command (state_t *state)
{
    int   ret;
    char *cmd;

    ret = get_popup_window_input  (">> ", state->input_buffer);
    if (ret == FAIL) {
        pfemr (ERR_POPUP_IN);
    }

    cmd = concatenate_strings (2, state->input_buffer, "\n");    
    ret = send_command_mp (state, cmd);
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }
    free (cmd);
    
    state->plugins[Dbg]->win->buff_data->new_data = true;
    state->plugins[Prg]->win->buff_data->new_data = true;
    ret = update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WINS);
    }

    return A_OK;
}



