#include "commands.h"

static int update_window_data(debugger_t *dbg, data_t *data)
{
    return A_OK;
}

int send_continue_cmd(debugger_t *dbg, data_t *data)
{
    switch (debugger_index(dbg)) {
        case GDB_DEBUGGER:
            if (send_command(dbg, "-exec-continue\n") == FAIL) 
                goto cmd_cont_err; 
            break;
        case PDB_DEBUGGER:
            break;
    }

    update_window_data(data, dbg);

    return A_OK;

cmd_cont_err:
    pfemr("Failed to send %s continue command", debugger_title(dbg));
}

int send_debugger_command (int plugin_index, state_t *state)
{
    bool  exiting = false;
    
    if (pulse_header_title_color (plugin_index, state, ON) == FAIL) 
        pfemr (ERR_PULSE_CMD DBG_CMD_STATE);

    switch (debugger_index) {
    case (DEBUGGER_GDB):
        switch (plugin_index) {
        case Con: if (send_command ("-exec-continue\n") == FAIL) goto dbg_cmd_err; break;
        case Fin: if (send_command ("-exec-finish\n") == FAIL) goto dbg_cmd_err; break;
        case Kil: if (send_command ("kill\n") == FAIL) goto dbg_cmd_err; break;
        case Nxi: if (send_command ("-exec-next-instruction\n") == FAIL) goto dbg_cmd_err; break;
        case Nxt: if (send_command ("-exec-next\n") == FAIL) goto dbg_cmd_err; break;
        case Run: 
            if (send_command ("-exec-run\n") == FAIL) goto dbg_cmd_err;
            if (file_was_updated (prog_update_time, prog_path))
                src_path_changed = true;
            new_run = true;
            break;
        case Sti: if (send_command ("-exec-step-instruction\n") == FAIL) goto dbg_cmd_err; break;
        case Stp: if (send_command ("step\n") == FAIL) goto dbg_cmd_err; break;
        case ESC:
        case Qut: if (send_command ("-gdb-exit\n") == FAIL) goto dbg_cmd_err; break;
        }
        break;
    case (DEBUGGER_PDB):
        switch (plugin_index) {
        case Con: if (send_command ("continue\n") == FAIL) goto dbg_cmd_err; break;
        case Fin: if (send_command ("return\n") == FAIL) goto dbg_cmd_err; break;
        case Kil: if (send_command ("restart\n") == FAIL) goto dbg_cmd_err; break;
        case Nxi: break;
        case Nxt: if (send_command ("next\n") == FAIL) goto dbg_cmd_err; break;
        case Run: if (send_command ("restart\n") == FAIL) goto dbg_cmd_err; break;
        case Sti: break;
        case Stp: if (send_command ("step\n") == FAIL) goto dbg_cmd_err; break;
        // FIX: Add ESC, Qut case
        }
    }

    if (plugin_index == Qut) {
        exiting = true;
        debugger_running = false;
    }

    if (!exiting) {

        // Flush GDB stdout
        if (debugger_index == DEBUGGER_GDB)
            if (send_command (state, "call ((void(*)(int))fflush)(0)\n") == FAIL)
                goto dbg_cmd_err;

        if (insert_output_end_marker() == FAIL)
            pfemr (ERR_OUT_MARK DBG_CMD_STATE);

        if (parse_debugger_output() == FAIL)
            pfemr (ERR_DBG_PARSE DBG_CMD_STATE);

        state->plugins[Dbg]->win->buff_data->new_data = true;
        state->plugins[Prg]->win->buff_data->new_data = true;

        if (update_windows (Dbg, Prg, Asm, Brk, LcV, Reg, Stk, Wat, Src) == FAIL)
            pfemr (ERR_UPDATE_WINS DBG_CMD_STATE);
        
        if (pulse_header_title_color (plugin_index, state, OFF) == FAIL)
            pfemr (ERR_PULSE_CMD DBG_CMD_STATE);
    }

    return A_OK;

dbg_cmd_err:
    pfemr (ERR_DBG_CMD DBG_CMD_STATE);
}
