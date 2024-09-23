#include "attach_to_process.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"

static void attach_to_process_gdb (state_t *state);


void
attach_to_process (state_t *state)
{
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            attach_to_process_gdb (state);
            break;
        case DEBUGGER_PDB:
            break;
    }
}



static void
attach_to_process_gdb (state_t *state)
{
    char *cmd,
         *cmd_base = "-target-attach ";

    get_popup_window_input  ("Attach to process ID or file: ", state->input_buffer);

    if (strlen (state->input_buffer) > 0) {

        cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");
        send_command_mp (state, cmd);
        free (cmd);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        update_windows (state, 9, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Stk, Wat);
    }
}
