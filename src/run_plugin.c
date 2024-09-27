#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"
#include "choose_layout.h"
#include "utilities.h"
#include "get_form_input/attach_to_process.h"
#include "get_form_input/run_custom_command.h"
#include "get_form_input/target_remote_server.h"
#include "get_form_input/execute_until.h"



int
run_plugin (int      plugin_index,
            state_t *state)
{
    int ret;

    switch (plugin_index) {

        // commands
        case Con:
        case Fin:
        case Kil:
        case Nxt:
        case Run:
        case Stp:
        case Qut:
            ret = send_debugger_command (plugin_index, state);
            if (ret == FAIL) {
                pfem ("FAIL: send_debugger_command (plugin_index, state)");
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
                ret = select_window (plugin_index, state);
                if (ret == FAIL) {
                    pfem ("FAIL: select_window (plugin_index, state)");
                    goto run_plugin_err;
                }
            }
            break;

        // popup window
        case AtP:
            ret = attach_to_process (state);
            if (ret == FAIL) {
                pfem ("FAIL: attach_to_process(state)");
                goto run_plugin_err;
            }
            break;
        case Lay: 
            ret = choose_layout (state);
            if (ret == FAIL) {
                pfem ("FAIL: choose_layout (state)");
                goto run_plugin_err;
            }
            break;
        case Prm: 
            ret = run_custom_command (state);
            if (ret == FAIL) {
                pfem ("FAIL: run_custom_command (state)");
                goto run_plugin_err;
            }
            break;
        case Trg: 
            ret = target_remote_server (state);
            if (ret == FAIL) {
                pfem ("FAIL: target_retmote_server (state)");
                goto run_plugin_err;
            }
            break;
        case Unt: 
            ret = execute_until (state);
            if (ret == FAIL) {
                pfem ("FAIL: execute_until (state)");
                goto run_plugin_err;
            }
            break;
    }

    return A_OK;

run_plugin_err:

    pemr ("plugin index: %d, code: \"%s\"", plugin_index, get_plugin_code (plugin_index));
}


