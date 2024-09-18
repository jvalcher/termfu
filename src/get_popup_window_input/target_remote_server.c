#include "target_remote_server.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"

static void target_remote_server_gdb (state_t *state);


void
target_remote_server (state_t *state)
{
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            target_remote_server_gdb (state);
            break;
    }
}



static void
target_remote_server_gdb (state_t *state)
{
    char *cmd,
         *cmd_base = "target remote ";

    get_popup_window_input  ("Target remote server: ", state->input_buffer);

    if (strlen (state->input_buffer) > 0) {

        cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");
        send_command_mp (state, cmd);
        free (cmd);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        update_windows (state, 9, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Stk, Wat);
    }
}
