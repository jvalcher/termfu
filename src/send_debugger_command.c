
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
    int debugger_index = state->debugger->index;

    pulse_header_title_color (plugin_index, state, ON);

    insert_output_start_marker (state);

    switch (plugin_index) {

        case Con:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "-exec-continue\n");
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "continue\n");
                    break;
            }
            break;

        case Fin:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "-exec-finish\n");
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "return\n");
                    break;
            }
            break;

        case Kil:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "kill\n");
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "restart\n");
                    break;
            }
            break;

        case Nxt:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "-exec-next\n");
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "next\n");
                    break;
            }
            break;

        case Stp:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "step\n");
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "step\n");
                    break;
            }
            break;

        case Run:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "-exec-run\n");
                    if (file_was_updated (state->debugger->prog_update_time, state->debugger->prog_path)) {
                        state->plugins[Src]->win->file_data->path_changed = true;
                    }
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "restart\n");
                    state->plugins[Src]->win->file_data->path_changed = true;
                    break;
            }
            break;

        case Qut:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    send_command (state, "-gdb-exit\n");
                    break;
                case (DEBUGGER_PDB):
                    send_command (state, "quit\n");
                    break;
            }
            state->debugger->running = false;
            exiting = true;
    }

    if (!exiting) {

        // flush program stdout
        switch (debugger_index) {
            case DEBUGGER_GDB:
                send_command (state, "call ((void(*)(int))fflush)(0)\n");
                break;
        }

        insert_output_end_marker (state);
        parse_debugger_output (state);

        // update Dbg, Prg windows using last parse
        // update Src next to provide data for remaining windows
        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        update_windows (state, 8, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Wat);
    }

    pulse_header_title_color (plugin_index, state, OFF);
}



