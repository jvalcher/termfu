
#include "execute_until.h"
#include "update_window_data/_update_window_data.h"
#include "get_popup_window_input/_get_popup_window_input.h"
#include "data.h"
#include "insert_output_marker.h"
#include "utilities.h"
#include "parse_debugger_output.h"
#include "update_window_data/_update_window_data.h"
#include "plugins.h"

static void execute_until_gdb (state_t *state);


void
execute_until (state_t *state)
{
    switch (state->debugger->curr) {
        case DEBUGGER_GDB:
            execute_until_gdb (state);
            break;
    }

    state->plugins[Dbg]->win->buff_data->new_data = true;
    state->plugins[Prg]->win->buff_data->new_data = true;
    update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat);
}



static void
execute_until_gdb (state_t *state)
{
    char *cmd_base = "-exec-until ";
    char *cmd;

    // TODO: get program CLI output

    get_popup_window_input  ("Execute until: ", state->input_buffer);

    cmd = concatenate_strings (3, cmd_base, state->input_buffer, "\n");    

    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    free (cmd);
}



