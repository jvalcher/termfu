#include <unistd.h>

#include "run_plugin.h"
#include "data.h"
#include "plugins.h"
#include "send_debugger_command.h"
#include "select_window.h"



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
        case Unt: 
        case Qut:
            send_debugger_command (plugin_index, state); 
            break;
                  
        // windows
        case Asm:
        case Brk:
        case LcV:
        case Out:
        case Prm:
        case Reg:
        case Src:
        case Wat: 
            if (state->plugins[plugin_index]->has_window) {
                select_window (plugin_index, state);
            }
            break;

        // popup windows
        case Bld:
        case Lay: 
            //TODO: open_popup_window (plugin_index, state);
            break;
    }
}


