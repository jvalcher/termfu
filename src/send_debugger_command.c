
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



int
send_debugger_command (int      plugin_index,
                       state_t *state)
{
    bool exiting = false;
    int debugger_index = state->debugger->index;

    // OPTIMIZE: make this a separate thread so that termfu can continue
    if (pulse_header_title_color (plugin_index, state, ON) == RET_FAIL) {
        pfem ("Failed to pulse header title color on");
        goto dbg_cmd_err_misc;
    }

    if (insert_output_start_marker (state) == RET_FAIL) {
        pfem ("Failed to insert output start marker");
        goto dbg_cmd_err_misc;
    }

    switch (plugin_index) {

        case Con:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "-exec-continue\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "continue\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
            }
            break;

        case Fin:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "-exec-finish\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "return\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
            }
            break;

        case Kil:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "kill\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "restart\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
            }
            break;

        case Nxt:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "-exec-next\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "next\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
            }
            break;

        case Run:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "-exec-run\n") == RET_FAIL) goto dbg_cmd_err;
                    if (file_was_updated (state->debugger->prog_update_time, state->debugger->prog_path)) {
                        state->plugins[Src]->win->src_file_data->path_changed = true;
                    }
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "restart\n") == RET_FAIL) goto dbg_cmd_err;
                    state->plugins[Src]->win->src_file_data->path_changed = true;
                    break;
            }
            break;

        case Stp:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "step\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "step\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
            }
            break;

        case Qut:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    if (send_command (state, "-gdb-exit\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
                case (DEBUGGER_PDB):
                    if (send_command (state, "quit\n") == RET_FAIL) goto dbg_cmd_err;
                    break;
            }
            state->debugger->running = false;
            exiting = true;
    }

    if (!exiting) {

        // flush program stdout
        switch (debugger_index) {
            case DEBUGGER_GDB:
                if (send_command (state, "call ((void(*)(int))fflush)(0)\n") == RET_FAIL) {
                    pfem ("Failed to send flush command");
                    goto dbg_cmd_err_misc;
                }
                break;
        }

        if (insert_output_end_marker (state) == RET_FAIL) {
            pfem ("Failed to insert output start marker");
            goto dbg_cmd_err_misc;
        }
        parse_debugger_output (state);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        if (update_windows (state, 9, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Stk, Wat) == RET_FAIL) {
            pfem ("Failed to update windows");
            goto dbg_cmd_err_misc;
        }
    }

    if (pulse_header_title_color (plugin_index, state, OFF) == RET_FAIL) {
        pfem ("Failed to pulse header title color off");
        goto dbg_cmd_err_misc;
    }

    return RET_OK;

dbg_cmd_err:

    switch (plugin_index) {
        case Con:
            pfem ("Failed to send continue command");
            break;
        case Fin:
            pfem ("Failed to send finish command");
            break;
        case Kil:
            pfem ("Failed to send kill command");
            break;
        case Nxt:
            pfem ("Failed to send next command");
            break;
        case Run:
            pfem ("Failed to send run command");
            break;
        case Stp:
            pfem ("Failed to send step command");
            break;
        case Qut:
            pfem ("Failed to send quit command");
            break;
    }

dbg_cmd_err_misc:

    pemr ("Send debugger (%s) command error (index: %d, code: %s)",
            state->debugger->title, plugin_index, get_plugin_code (plugin_index));
}



