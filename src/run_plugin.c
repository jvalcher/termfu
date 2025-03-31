#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"
#include "choose_layout.h"
#include "error.h"
#include "get_form_input/run_prompt_command.h"
#include "get_form_input/execute_until.h"
#include "get_form_input/attach_to_process.h"

#define RUN_PLUG_STATE  " (plugin index: %d, code: \"%s\")", plugin_index, get_plugin_code (plugin_index)



int
run_plugin (int      plugin_index,
            state_t *state)
{
    state->debugger->running_plugin = true;
    state->debugger->curr_plugin_index = plugin_index;

    switch (plugin_index) {

    // commands
    case Con:
    case Fin:
    case Kil:
    case Nxt:
    case Nxi:
    case Run:
    case Stp:
    case Sti:
    case Qut:
        if (send_debugger_command (plugin_index, state) == FAIL)
            pfemr ("Failed to send debugger command" RUN_PLUG_STATE);
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
        if (state->plugins[plugin_index]->has_window)
            if (select_window (plugin_index, state) == FAIL)
                pfemr ("Failed to select window" RUN_PLUG_STATE);
        break;

    // form input
    case Lay: 
        if (choose_layout (state) == FAIL)
            pfemr ("Failed to choose layout" RUN_PLUG_STATE);
        break;
    case Prm: 
        if (run_prompt_command (state) == FAIL)
            pfemr ("Failed to run custom prompt command" RUN_PLUG_STATE);
        break;
    case Unt: 
        if (execute_until (state) == FAIL)
            pfemr ("Failed to execute until" RUN_PLUG_STATE);
        break;

    // other
    case AtP:
        if (attach_to_process (state) == FAIL)
            pfemr ("Failed to attach to debugged process" RUN_PLUG_STATE);
        break;
    }

    state->debugger->running_plugin = false;

    return A_OK;
}
