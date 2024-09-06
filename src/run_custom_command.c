#include "get_popup_window_input/_get_popup_window_input.h"
#include "data.h"
#include "insert_output_marker.h"
#include "utilities.h"
#include "parse_debugger_output.h"
#include "plugins.h"
#include "update_window_data/_update_window_data.h"



void
run_custom_command (state_t *state)
{
    char *cmd;

    get_popup_window_input  (">> ", state->input_buffer);

    cmd = concatenate_strings (2, state->input_buffer, "\n");    

    insert_output_start_marker (state);
    send_command (state, cmd);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    free (cmd);
    
    state->plugins[Dbg]->win->buff_data->new_data = true;
    state->plugins[Prg]->win->buff_data->new_data = true;
    update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat);
}



