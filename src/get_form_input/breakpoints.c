#include "breakpoints.h"
#include "_get_form_input.h"
#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"


// TODO: make ALL breakpoints persist
// Currently breakpoints that return an error are ignored
// Change creation process to be more like watchpoints
// Breakpoints that don't exist in current program show a 'none' value

int
insert_breakpoint (state_t *state)
{
    char *cmd_base_gdb = "-break-insert ",
         *cmd_base_pdb = "break ",
         *cmd_base = NULL;

    if (get_form_input ("Insert breakpoint: ", state->input_buffer) == FAIL)
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

        if (update_windows (Brk, Src)  == FAIL)
            pfemr (ERR_UPDATE_WIN);
    }

    return A_OK;
}



int
delete_breakpoint (state_t *state)
{
    char *cmd_base_gdb = "-break-delete ",
         *cmd_base_pdb = "clear ",
         *cmd_base = NULL;

    if (get_form_input ("Delete breakpoint: ", state->input_buffer) == FAIL)
        pfemr (ERR_POPUP_IN);

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            cmd_base = (char*) cmd_base_gdb;
            break;
        case DEBUGGER_PDB:
            cmd_base = (char*) cmd_base_pdb;
            break;
    }

    if (send_command_mp (state, cmd_base, state->input_buffer, "\n") == FAIL)
        pfemr (ERR_DBG_CMD);

    if (update_windows (Brk, Src) == FAIL)
        pfemr (ERR_UPDATE_WIN);

    return A_OK;
}



int
clear_all_breakpoints (state_t *state)
{
    breakpoint_t *curr_break;
    char *cmd_base_gdb = "-break-delete ",
         *cmd_base_pdb = "clear ",
         *cmd_base = NULL;

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
            
            if (send_command_mp (state, cmd_base, curr_break->index, "\n") == FAIL )
                pfemr (ERR_DBG_CMD);

            curr_break = curr_break->next;  
        } while (curr_break != NULL);
    }
    state->breakpoints = NULL;

    if (update_windows (Brk, Src) == FAIL)
        pfemr (ERR_UPDATE_WIN);

    return A_OK;
}

