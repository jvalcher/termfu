
#include "execute_until.h"
#include "../update_window_data/_update_window_data.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"



void
execute_until (state_t *state)
{
    char *cmd_base_gdb = "-exec-until ",
         *cmd_base_pdb = "until ",
         *cmd_base,
         *cmd;

    get_popup_window_input  ("Execute until (line, filename:line): ", state->input_buffer);

    if (strlen (state->input_buffer) > 0) {

        switch (state->debugger->index) {
            case DEBUGGER_GDB:
                cmd_base = cmd_base_gdb;
                break;
            case DEBUGGER_PDB:
                cmd_base = cmd_base_pdb;
                break;
        }
        cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    

        send_command_mp (state, cmd);

        free (cmd);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat);
    }
}
