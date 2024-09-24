#include "target_remote_server.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"

static int target_remote_server_gdb (state_t *state);


int
target_remote_server (state_t *state)
{
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            if (target_remote_server_gdb (state) == RET_FAIL) {
                pfemr ("Failed to target remote server (GDB)");
            }
            break;
        case DEBUGGER_PDB:
            break;
    }

    return RET_OK;
}



static int
target_remote_server_gdb (state_t *state)
{
    char *cmd,
         *cmd_base = "target remote ";

    if (get_popup_window_input  ("Target remote server: ", state->input_buffer) == RET_FAIL) {
        pfemr ("Failed to get target remote input");
    }

    if (strlen (state->input_buffer) > 0) {

        cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");
        if (send_command_mp (state, cmd) == RET_FAIL) {
            pfemr ("Failed to send target remote command");
        }
        free (cmd);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        if (update_windows (state, 9, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Stk, Wat) == RET_FAIL) {
            pfemr ("Failed to update windows");
        }
    }

    return RET_OK;
}
