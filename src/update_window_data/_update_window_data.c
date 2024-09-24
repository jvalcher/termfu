#include <stdarg.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../display_lines.h"
#include "../utilities.h"

#include "get_assembly_data.h"
#include "get_breakpoint_data.h"
#include "get_debugger_output.h"
#include "get_local_vars.h"
#include "get_program_output.h"
#include "get_register_data.h"
#include "get_source_path_line_memory.h"
#include "get_stack_data.h"
#include "get_watchpoint_data.h"

int update_window (int, state_t*);



int
update_windows (state_t *state,
                int num_updates,
                ...)
{
    int plugin;
    va_list  plugins;

    va_start (plugins, num_updates);

    for (int i = 0; i < num_updates; i++) {
        plugin = va_arg (plugins, int);
        if (update_window (plugin, state) == RET_FAIL) {
            pfemr ("Update window loop failed");
        }
    }

    va_end (plugins);

    return RET_OK;
}



int
update_window (int      plugin_index,
               state_t *state)
{
    switch (plugin_index) {
        case Asm:
            if (get_assembly_data (state) == RET_FAIL) {
                pfem ("Failed to get assembly data");
                goto upd_win_err;
            }
            break;
        case Brk: 
            if (get_breakpoint_data (state) == RET_FAIL) {
                pfem ("Failed to get breakpoint data");
                goto upd_win_err;
            }
            break;
        case Dbg:
            if (get_debugger_output (state) == RET_FAIL) {
                pfem ("Failed to get debugger output");
                goto upd_win_err;
            }
            break;
        case LcV:
            if (get_local_vars (state) == RET_FAIL) {
                pfem ("Failed to get local variables");
                goto upd_win_err;
            }
            break;
        case Prg:
            if (get_program_output (state) == RET_FAIL) {
                pfem ("Failed to get program output");
                goto upd_win_err;
            }
            break;
        case Reg:
            if (get_register_data (state) == RET_FAIL) {
                pfem ("Failed to get register data");
                goto upd_win_err;
            }
            break;
        case Src:
            if (get_source_path_line_memory (state) == RET_FAIL) {
                pfem ("Failed to get source, line, memory data");
                goto upd_win_err;
            }
            break;
        case Stk:
            if (get_stack_data (state) == RET_FAIL) {
                pfem ("Failed to get stack data");
                goto upd_win_err;
            }
            break;
        case Wat:
            if (get_watchpoint_data (state) == RET_FAIL) {
                pfem ("Failed to get watchpoint data");
                goto upd_win_err;
            }
            break;
        default:
            pfemr ("Unrecognized plugin index");
            goto upd_win_err;
    }

    if (display_lines (state->plugins[plugin_index]->win_type,
                       state->plugins[plugin_index]->data_pos,
                       plugin_index,
                       state) == RET_FAIL)
    {
        pfem ("Failed to display lines (type: %d, pos: %d)",
                state->plugins[plugin_index]->win_type,
                state->plugins[plugin_index]->data_pos);
    }

    return RET_OK;

upd_win_err:

    pemr ("Failed to update window (index: %d, code: %s)",
            plugin_index, get_plugin_code (plugin_index));
}

