#include "attach_to_process.h"
#include "../data.h"
#include "../utilities.h"
#include "../get_popup_window_input/_get_popup_window_input.h"
#include "../plugins.h"
#include "../update_window_data/_update_window_data.h"

static int attach_to_process_gdb (state_t *state);



int
attach_to_process (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            ret = attach_to_process_gdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to attach to process");
            }
            break;
        case DEBUGGER_PDB:
            break;
    }

    return A_OK;
}



static int
attach_to_process_gdb (state_t *state)
{
    int   ret;
    char *cmd,
         *cmd_base = "-target-attach ";

    ret = get_popup_window_input  ("Attach to process ID or file: ", state->input_buffer);
    if (ret == FAIL) {
        pfemr (ERR_POPUP_IN);
    }

    if (strlen (state->input_buffer) > 0) {

        cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");
        ret = send_command_mp (state, cmd);
        if (ret == FAIL) {
            pfemr (ERR_DBG_CMD);
        }
        free (cmd);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        ret = update_windows (state, 9, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Stk, Wat);
        if (ret == FAIL) {
            pfemr (ERR_UPDATE_WINS);
        }
    }

    return A_OK;
}
