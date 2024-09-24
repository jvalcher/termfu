#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"
#include "choose_layout.h"
#include "utilities.h"
#include "get_popup_window_input/attach_to_process.h"
#include "get_popup_window_input/run_custom_command.h"
#include "get_popup_window_input/target_remote_server.h"
#include "get_popup_window_input/execute_until.h"



int
run_plugin (int      plugin_index,
            state_t *state)
{
    switch (plugin_index) {

        // commands
        case Con:
        case Fin:
        case Kil:
        case Nxt:
        case Run:
        case Stp:
        case Qut:
            if (send_debugger_command (plugin_index, state) == RET_FAIL) {
                pfem ("Failed to send debugger command");
                goto run_plugin_err;
            }
            break;
                  
        // window loop
        case Asm:
        case Brk:
        case Dbg:
        case LcV:
        case Prg:
        case Reg:
        case Src:
        case Stk:
        case Wat: 
            if (state->plugins[plugin_index]->has_window) {
                if (select_window (plugin_index, state) == RET_FAIL) {
                    pfem ("Failed to select window");
                    goto run_plugin_err;
                }
            }
            break;

        // popup window
        case AtP:
            if (attach_to_process (state) == RET_FAIL) {
                pfem ("Failed to attach to process");
                goto run_plugin_err;
            }
            break;
        case Lay: 
            if (choose_layout (state) == RET_FAIL) {
                pfem ("Failed to choose layout");
                goto run_plugin_err;
            }
            break;
        case Prm: 
            if (run_custom_command (state) == RET_FAIL) {
                pfem ("Failed to run custom command");
                goto run_plugin_err;
            }
            break;
        case Trg: 
            if (target_remote_server (state) == RET_FAIL) {
                pfem ("Failed to target remote server");
                goto run_plugin_err;
            }
            break;
        case Unt: 
            if (execute_until (state) == RET_FAIL) {
                pfem ("Failed to execute until");
                goto run_plugin_err;
            }
            break;
    }

    return RET_OK;

run_plugin_err:

    pemr ("run_plugin error (index: %d, code: %s)",
                plugin_index, get_plugin_code (plugin_index));
}


