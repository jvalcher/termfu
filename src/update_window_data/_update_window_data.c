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
#include "get_source_path_line_func.h"
#include "get_stack_data.h"
#include "get_watchpoint_data.h"
#include "../persist_data.h"



int
update_window (int      plugin_index,
               state_t *state)
{
    int ret;

    // get data
    switch (plugin_index) {
        case Asm:
            ret = get_assembly_data (state);
            if (ret == FAIL) {
                pfem ("Failed to get assembly data");
                goto upd_win_err;
            }
            break;
        case Brk: 
            ret = get_breakpoint_data (state);
            if (ret == FAIL) {
                pfem ("Failed to get breakpoint data");
                goto upd_win_err;
            }
            break;
        case Dbg:
            ret = get_debugger_output (state);
            if (ret == FAIL) {
                pfem ("Failed to get debugger output");
                goto upd_win_err;
            }
            break;
        case LcV:
            ret = get_local_vars (state);
            if (ret == FAIL) {
                pfem ("Failed to get local variables");
                goto upd_win_err;
            }
            break;
        case Prg:
            ret = get_program_output (state);
            if (ret == FAIL) {
                pfem ("Failed to get program output");
                goto upd_win_err;
            }
            break;
        case Reg:
            ret = get_register_data (state);
            if (ret == FAIL) {
                pfem ("Failed to get register data");
                goto upd_win_err;
            }
            break;
        case Src:
            ret = get_source_path_line_func (state);
            if (ret == FAIL) {
                pfem ("Failed to get source, line, memory data");
                goto upd_win_err;
            }
            break;
        case Stk:
            ret = get_stack_data (state);
            if (ret == FAIL) {
                pfem ("Failed to get stack data");
                goto upd_win_err;
            }
            break;
        case Wat:
            ret = get_watchpoint_data (state);
            if (ret == FAIL) {
                pfem ("Failed to get watchpoint data");
                goto upd_win_err;
            }
            break;
        default:
            pfem ("Unrecognized plugin index");
            goto upd_win_err;
    }

    // update window
    ret = display_lines (state->plugins[plugin_index]->data_pos,
                         plugin_index,
                         state);
    if (ret == FAIL) {
        pfem (ERR_DISP_LINES);
        goto upd_win_err;
    }

    // persist data
    switch (plugin_index) {
        case Brk:
        case Wat:
            ret = persist_data (state);
            if (ret == FAIL) {
                pfeme ("Failed to persist data");
            }
    }

    return A_OK;

upd_win_err:

    pemr ("Failed to update window (index: %d, code: \"%s\", debugger: \"%s\")",
            plugin_index, get_plugin_code (plugin_index), state->debugger->title);
}



int
update_windows (state_t *state,
                int num_updates,
                ...)
{
    int      i,
             plugin_index,
             ret;
    va_list  plugins;

    va_start (plugins, num_updates);
    for (i = 0; i < num_updates; i++) {
        plugin_index = va_arg (plugins, int);
        ret = update_window (plugin_index, state);
        if (ret == FAIL) {
            pfemr ("Update window loop failed");
        }
    }
    va_end (plugins);

    return A_OK;
}

