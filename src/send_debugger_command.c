
/*
    Interface for plugin and other debugger-specific functions
*/


#include "send_debugger_command.h"
#include "data.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"
#include "plugins.h"
#include "utilities.h"
#include "pulse_header_title_color.h"
#include "update_window_data/_update_window_data.h"



void
send_debugger_command (int      plugin_index,
                       state_t *state)
{
    bool exiting = false;
    int curr_debugger = state->debugger->curr;

    pulse_header_title_color (plugin_index, state, ON);

    insert_output_start_marker (state);

    switch (plugin_index) {
    case Con:
        switch (curr_debugger) {
        case (DEBUGGER_GDB):
            send_command (state, "-exec-continue\n");
            break;
        }
        break;
    case Fin:
        switch (curr_debugger) {
        case (DEBUGGER_GDB):
            send_command (state, "-exec-finish\n");
            break;
        }
        break;
    case Kil:
        switch (curr_debugger) {
        case (DEBUGGER_GDB):
            send_command (state, "kill\n");
            break;
        }
        break;
    case Nxt:
        switch (curr_debugger) {
        case (DEBUGGER_GDB):
            send_command (state, "-exec-next\n");
            break;
        }
        break;
    case Stp:
        switch (curr_debugger) {
        case (DEBUGGER_GDB):
            send_command (state, "step\n");
            break;
        }
        break;
    case Run:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): 
            send_command (state, "-exec-run\n");
            break;
        }
        break;
    case Qut:
        switch (curr_debugger) {
        case (DEBUGGER_GDB):
            send_command (state, "-gdb-exit\n");
        }
        state->debugger->running = false;
        exiting = true;
    }

    if (!exiting) {

        // flush program stdout
        switch (curr_debugger) {
        case DEBUGGER_GDB:
            send_command (state, "call ((void(*)(int))fflush)(0)\n");
            break;
        }

        insert_output_end_marker (state);
        parse_debugger_output (state);

        // update Dbg, Prg windows using last parse
        // update Src first to update info
        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat);
    }

    pulse_header_title_color (plugin_index, state, OFF);
}



