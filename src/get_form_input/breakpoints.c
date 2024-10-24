
#include "breakpoints.h"
#include "_get_form_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"


// TODO: store, persist invalid breakpoints

int
insert_breakpoint (state_t *state)
{
    int   ret;
    char *cmd_base_gdb = "-break-insert ",
         *cmd_base_pdb = "break ",
         *cmd_base,
         *cmd;

    ret = get_form_input ("Insert breakpoint: ", state->input_buffer);
    if (ret == FAIL) {
        pfemr (ERR_POPUP_IN);
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
    ret = send_command_mp (state, cmd);
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }
    free (cmd);

    ret = update_window (Brk, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    return A_OK;
}



int
delete_breakpoint (state_t *state)
{
    int   ret;
    char *cmd_base_gdb = "-break-delete ",
         *cmd_base_pdb = "clear ",
         *cmd_base,
         *cmd;

    ret = get_form_input ("Delete breakpoint: ", state->input_buffer);
    if (ret == FAIL) {
        pfemr (ERR_POPUP_IN);
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
    ret = send_command_mp (state, cmd);
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }
    free (cmd);

    ret = update_window (Brk, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    return A_OK;
}



int
clear_all_breakpoints (state_t *state)
{
    breakpoint_t *curr_break;
    int   ret;
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
            ret = send_command_mp (state, cmd);
            if (ret == FAIL) {
                pfemr (ERR_DBG_CMD);
            }
            free (cmd);

            curr_break = curr_break->next;  
        } while (curr_break != NULL);
    }
    state->breakpoints = NULL;

    ret = update_window (Brk, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    return A_OK;
}

