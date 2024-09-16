
#include "popup_breakpoints.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../utilities.h"
#include "../parse_debugger_output.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"



void
insert_breakpoint (state_t *state)
{
    char *cmd_base_gdb = "-break-insert ",
         *cmd_base_pdb = "break ",
         *cmd_base,
         *cmd;

    get_popup_window_input  ("Insert breakpoint: ", state->input_buffer);

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

    update_window (Brk, state);
}



void
delete_breakpoint (state_t *state)
{
    char *cmd_base_gdb = "-break-delete ",
         *cmd_base_pdb = "clear ",
         *cmd_base,
         *cmd;

    get_popup_window_input  ("Delete breakpoint: ", state->input_buffer);

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            cmd_base = (char*) cmd_base_gdb;
            break;
        case DEBUGGER_PDB:
            cmd_base = (char*) cmd_base_pdb;
            break;
    }

    cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    

    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    free (cmd);

    update_window (Brk, state);
}

