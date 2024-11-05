#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"
#include "choose_layout.h"
#include "utilities.h"
#include "get_form_input/run_custom_command.h"
#include "get_form_input/execute_until.h"
#include "get_form_input/attach_to_process.h"



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
                ret = select_window (plugin_index, state);
                if (ret == FAIL) {
                    pfem ("Failed to select window");
                    goto run_plugin_err;
                }
            }
            break;

        case Lay: 
            ret = choose_layout (state);
            if (ret == FAIL) {
                pfem ("Failed to choose layout");
                goto run_plugin_err;
            }
            break;
        case Prm: 
            ret = run_custom_command (state);
            if (ret == FAIL) {
                pfem ("Failed to run custom prompt command");
                goto run_plugin_err;
            }
            break;
        case Unt: 
            ret = execute_until (state);
            if (ret == FAIL) {
                pfem ("Failed to execute until");
                goto run_plugin_err;
            }
            break;


        // other
        case AtP:
            ret = attach_to_process (state);
            if (ret == FAIL) {
                pfem ("Failed to attach to debugged process");
                goto run_plugin_err;
            }
            break;
    }

    return A_OK;

run_plugin_err:

    pemr ("plugin index: %d, code: \"%s\"", plugin_index, get_plugin_code (plugin_index));
}


