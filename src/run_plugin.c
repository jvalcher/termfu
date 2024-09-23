#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"
#include "choose_layout.h"
#include "get_popup_window_input/attach_to_process.h"
#include "get_popup_window_input/run_custom_command.h"
#include "get_popup_window_input/target_remote_server.h"
#include "get_popup_window_input/execute_until.h"



void
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
            send_debugger_command (plugin_index, state); 
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
                select_window (plugin_index, state);
            }
            break;

        // popup window
        case AtP:
            attach_to_process (state);
            break;
        case Lay: 
            choose_layout (state);
            break;
        case Prm: 
            run_custom_command (state);
            break;
        case Trg: 
            target_remote_server (state);
            break;
        case Unt: 
            execute_until (state);
            break;
    }
}


