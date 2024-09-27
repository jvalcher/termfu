#include "target_remote_server.h"
#include "_get_form_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"

static int target_remote_server_gdb (state_t *state);



int
target_remote_server (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            ret = target_remote_server_gdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to target remote server (GDB)");
            }
            break;
        case DEBUGGER_PDB:
            break;
    }

    return A_OK;
}



static int
target_remote_server_gdb (state_t *state)
{
    int   ret;
    char *cmd,
         *cmd_base = "target remote ";

    ret = get_form_input ("Target remote server: ", state->input_buffer);
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
