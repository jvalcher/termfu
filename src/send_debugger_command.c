#include "send_debugger_command.h"
#include "data.h"
#include "parse_debugger_output.h"
#include "plugins.h"
#include "utilities.h"
#include "error.h"
#include "pulse_header_title_color.h"
#include "update_window_data/_update_window_data.h"


#define DBG_CMD_STATE  " (debugger: \"%s\", plugin index: %d, code: \"%s\")", \
                       state->debugger->title, plugin_index, get_plugin_code (plugin_index)

// TODO: Change back to switch statements
// define macro function for "Failed to send <cmd> command"
#define F(cmd)  pfemr ("Failed to send " #cmd " command"


const char *gdb_cmds[] = {
    NULL,               // Asm
    NULL,               // AtP
    NULL,               // Brk
    "-exec-continue\n", // Con
    NULL,               // Dbg
    "-exec-finish\n",   // Fin
    "kill\n",           // Kil
    NULL,               // Lay
    NULL,               // LcV
    "-exec-next\n",     // Nxt
    NULL,               // Prg
    NULL,               // Prm
    "-gdb-exit\n",      // Qut
    NULL,               // Reg
    "-exec-run\n",      // Run
    NULL,               // Src
    NULL,               // Stk
    "step\n",           // Stp
    NULL,               // Unt
    NULL,               // Wat
};

const char *pdb_cmds[] = {
    NULL,               // Asm
    NULL,               // AtP
    NULL,               // Brk
    "continue\n",       // Con
    NULL,               // Dbg
    "return\n",         // Fin
    "restart\n",        // Kil
    NULL,               // Lay
    NULL,               // LcV
    "next\n",           // Nxt
    NULL,               // Prg
    NULL,               // Prm
    "quit\n",           // Qut
    NULL,               // Reg
    "restart\n",        // Run
    NULL,               // Src
    NULL,               // Stk
    "step\n",           // Stp
    NULL,               // Unt
    NULL,               // Wat
};



int
send_debugger_command (int      plugin_index,
                       state_t *state)
{
    bool  exiting = false;
    int   debugger_index = state->debugger->index;
    
    if (pulse_header_title_color (plugin_index, state, ON) == FAIL)
        pfemr (ERR_PULSE_CMD DBG_CMD_STATE);

    switch (debugger_index) {
    case (DEBUGGER_GDB):
        switch (plugin_index) {
        case Fin: if (send_command (state, "-exec-finish\n") == FAIL) goto dbg_cmd_err; break;
        case Kil: if (send_command (state, "kill\n") == FAIL) goto dbg_cmd_err; break;
        case Nxi: if (send_command (state, "-exec-next-instruction\n") == FAIL) goto dbg_cmd_err; break;
        case Nxt: if (send_command (state, "-exec-next\n") == FAIL) goto dbg_cmd_err; break;
        case Run: 
            if (send_command (state, "-exec-run\n") == FAIL) goto dbg_cmd_err;
            if (file_was_updated (state->debugger->prog_update_time, state->debugger->prog_path))
                state->debugger->src_path_changed = true;
            state->new_run = true;
            break;
        case Sti: if (send_command (state, "-exec-step-instruction\n") == FAIL) goto dbg_cmd_err; break;
        case Stp: if (send_command (state, "step\n") == FAIL) goto dbg_cmd_err; break;
        }
        break;
    case (DEBUGGER_PDB):
        switch (plugin_index) {
        case Fin: if (send_command (state, "return\n") == FAIL) goto dbg_cmd_err; break;
        case Kil: if (send_command (state, "restart\n") == FAIL) goto dbg_cmd_err; break;
        case Nxi: break;
        case Nxt: if (send_command (state, "next\n") == FAIL) goto dbg_cmd_err; break;
        case Run: if (send_command (state, "restart\n") == FAIL) goto dbg_cmd_err; break;
        case Sti: break;
        case Stp: if (send_command (state, "step\n") == FAIL) goto dbg_cmd_err; break;
        case ESC:
        case Qut: if (send_command (state, "-gdb-exit\n") == FAIL) goto dbg_cmd_err; break;
        }
    }

    if (plugin_index == Qut) {
        exiting = true;
        state->debugger->running = false;
    }

    if (!exiting) {

        // Flush GDB stdout
        if (debugger_index == DEBUGGER_GDB)
            if (send_command (state, "call ((void(*)(int))fflush)(0)\n") == FAIL)
                goto dbg_cmd_err;

        if (insert_output_end_marker (state) == FAIL)
            pfemr (ERR_OUT_MARK DBG_CMD_STATE);

        if (parse_debugger_output (state) == FAIL)
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
