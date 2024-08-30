
#include "popup_breakpoints.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../utilities.h"
#include "../parse_debugger_output.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"

static void insert_breakpoint_gdb (state_t *state);
static void delete_breakpoint_gdb (state_t *state);


void
insert_breakpoint (state_t *state)
{
    switch (state->debugger->curr) {
        case DEBUGGER_GDB:
            insert_breakpoint_gdb (state);
            break;
    }
    update_window (Brk, state);
}



void
delete_breakpoint (state_t *state)
{
    switch (state->debugger->curr) {
        case DEBUGGER_GDB:
            delete_breakpoint_gdb (state);
            break;
    }
    update_window (Brk, state);
}



static void
insert_breakpoint_gdb (state_t *state)
{
    char *cmd_base = "-break-insert ";
    char *cmd;

    get_popup_window_input  ("Insert breakpoint: ", state->input_buffer);

    cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    

    // send breakpoint command
    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    free (cmd);
}



static void
delete_breakpoint_gdb (state_t *state)
{
    char *cmd_base = "-break-delete ";
    char *cmd;

    get_popup_window_input  ("Delete breakpoint: ", state->input_buffer);

    cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    

    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    free (cmd);
}

