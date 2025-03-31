
#include "execute_until.h"
#include "_get_form_input.h"
#include "../update_window_data/_update_window_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"



int
execute_until (state_t *state)
{
    char *cmd_base_gdb = "-exec-until ",
         *cmd_base_pdb = "until ",
         *cmd_base = NULL;

    if (get_form_input ("Execute until line or filename:line: ", state->input_buffer) == FAIL)
        pfemr (ERR_POPUP_IN);

    if (strlen (state->input_buffer) > 0) {

        switch (state->debugger->index) {
            case DEBUGGER_GDB:
                cmd_base = cmd_base_gdb;
                break;
            case DEBUGGER_PDB:
                cmd_base = cmd_base_pdb;
                break;
        }
        if (send_command_mp (state, cmd_base, state->input_buffer, "\n") == FAIL)
            pfemr (ERR_DBG_CMD);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;

        if (update_windows (Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat) == FAIL)
            pfemr (ERR_UPDATE_WINS);
    }

    return A_OK;
}

