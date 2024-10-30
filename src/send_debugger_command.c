
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
    int ret,
        debugger_index = state->debugger->index;

    ret = pulse_header_title_color (plugin_index, state, ON);
    if (ret == FAIL) {
        pfem (ERR_PULSE_CMD);
        goto dbg_cmd_err_end;
    }

    ret = insert_output_start_marker (state);
    if (ret == FAIL) {
        pfem (ERR_OUT_MARK);
        goto dbg_cmd_err_end;
    }

    switch (plugin_index) {

        case Con:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "-exec-continue\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "continue\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
            }
            break;

        case Fin:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "-exec-finish\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "return\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
            }
            break;

        case Kil:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "kill\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "restart\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
            }
            break;

        case Nxt:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "-exec-next\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "next\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
            }
            break;

        case Run:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "-exec-run\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    if (file_was_updated (state->debugger->prog_update_time,
                                          state->debugger->prog_path))
                    {
                        state->debugger->src_path_changed = true;
                    }
                    state->new_run = true;
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "restart\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    state->debugger->src_path_changed = true;
                    break;
            }
            break;

        case Stp:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "step\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "step\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
            }
            break;

        case ESC:
        case Qut:
            switch (debugger_index) {
                case (DEBUGGER_GDB):
                    ret = send_command (state, "-gdb-exit\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
                case (DEBUGGER_PDB):
                    ret = send_command (state, "quit\n");
                    if (ret == FAIL) {
                        goto dbg_cmd_err;
                    }
                    break;
            }
            state->debugger->running = false;
            exiting = true;
    }

    if (!exiting) {

        // flush program stdout
        switch (debugger_index) {
            case DEBUGGER_GDB:
                ret = send_command (state, "call ((void(*)(int))fflush)(0)\n");
                if (ret == FAIL) {
                    pfem (ERR_DBG_CMD);
                    goto dbg_cmd_err_end;
                }
                break;
        }

        ret = insert_output_end_marker (state);
        if (ret == FAIL) {
            pfem (ERR_OUT_MARK);
            goto dbg_cmd_err_end;
        }

        ret = parse_debugger_output (state);
        if (ret == FAIL) {
            pfem (ERR_DBG_PARSE);
            goto dbg_cmd_err_end;
        }

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;
        ret = update_windows (state, 9, Dbg, Prg, Asm, Brk, LcV, Reg, Stk, Wat, Src);
        if (ret == FAIL) {
            pfem (ERR_UPDATE_WINS);
            goto dbg_cmd_err_end;
        }
    }

    ret = pulse_header_title_color (plugin_index, state, OFF);
    if (ret == FAIL) {
        pfem (ERR_PULSE_CMD);
        goto dbg_cmd_err_end;
    }

    return A_OK;

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

dbg_cmd_err_end:

    pemr ("Send debugger (%s) command error (index: %d, code: %s)",
            state->debugger->title, plugin_index, get_plugin_code (plugin_index));
}



