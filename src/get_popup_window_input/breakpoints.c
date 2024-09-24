
#include "breakpoints.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"



int
insert_breakpoint (state_t *state)
{
    char *cmd_base_gdb = "-break-insert ",
         *cmd_base_pdb = "break ",
         *cmd_base,
         *cmd;

    if (get_popup_window_input  ("Insert breakpoint: ", state->input_buffer) == RET_FAIL) {
        pfemr ("Failed to get popup window input");
    }

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            cmd_base = cmd_base_gdb;
            break;
        case DEBUGGER_PDB:
            cmd_base = cmd_base_pdb;
            break;
    }

    cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    
    if (send_command_mp (state, cmd) == RET_FAIL) {
        pfemr ("Failed to send insert breakpoint command");
    }
    free (cmd);

    if (update_window (Brk, state) == RET_FAIL) {
        pfemr ("Failed to update breakpoint window");
    }

    return RET_OK;
}



int
delete_breakpoint (state_t *state)
{
    char *cmd_base_gdb = "-break-delete ",
         *cmd_base_pdb = "clear ",
         *cmd_base,
         *cmd;

    if (get_popup_window_input  ("Delete breakpoint: ", state->input_buffer) == RET_FAIL) {
        pfemr ("Failed to get delete breakpoint input");
    }

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            cmd_base = (char*) cmd_base_gdb;
            break;
        case DEBUGGER_PDB:
            cmd_base = (char*) cmd_base_pdb;
            break;
    }

    cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    
    if (send_command_mp (state, cmd) == RET_FAIL) {
        pfemr ("Failed to send delete breakpoint command");
    }
    free (cmd);

    update_window (Brk, state);

    return RET_OK;
}



int
clear_all_breakpoints (state_t *state)
{
    breakpoint_t *curr_break;
    char *cmd_base_gdb = "-break-delete ",
         *cmd_base_pdb = "clear ",
         *cmd_base,
         *cmd;

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            cmd_base = (char*) cmd_base_gdb;
            break;
        case DEBUGGER_PDB:
            cmd_base = (char*) cmd_base_pdb;
            break;
    }

    if (state->breakpoints != NULL) {
        curr_break = state->breakpoints;
        do {
            
            cmd = concatenate_strings (3, cmd_base, curr_break->index, "\n");    
            if (send_command_mp (state, cmd) == RET_FAIL) {
                pfemr ("Failed to send delete breakpoint command");
            }
            free (cmd);

            curr_break = curr_break->next;  
        } while (curr_break != NULL);
    }
    state->breakpoints = NULL;

    if (update_window (Brk, state) == RET_FAIL) {
        pfemr ("Failed to update breakpoint window");
    }

    return RET_OK;
}

