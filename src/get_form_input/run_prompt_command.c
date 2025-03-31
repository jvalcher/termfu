#include "run_prompt_command.h"
#include "_get_form_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../plugins.h"
#include "../update_window_data/_update_window_data.h"


// TODO: save commands, up/down key access

int
run_prompt_command (state_t *state)
{
    char *prompt;

    prompt = concatenate_strings ("(", state->debugger->title, ")");
    if (get_form_input (prompt, state->input_buffer) == FAIL)
        pfemr (ERR_POPUP_IN);
    free (prompt);

    if (strlen (state->input_buffer) > 0) {

        if (send_command_mp (state, state->input_buffer, "\n") == FAIL)
            pfemr (ERR_DBG_CMD);
        
        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;

        if (update_windows (Dbg, Prg, Asm, Brk, LcV, Reg, Wat, Src) == FAIL)
            pfemr (ERR_UPDATE_WINS);
    }

    return A_OK;
}



