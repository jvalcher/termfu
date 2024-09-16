#include <stdarg.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../display_lines.h"

#include "get_assembly_data.h"
#include "get_breakpoint_data.h"
#include "get_debugger_output.h"
#include "get_local_vars.h"
#include "get_program_output.h"
#include "get_register_data.h"
#include "get_source_path_line_memory.h"
#include "get_watchpoint_data.h"

void  update_window  (int, state_t*);



void
update_windows (state_t *state,
                int num_updates,
                ...)
{
    int plugin;
    va_list  plugins;

    va_start (plugins, num_updates);

    for (int i = 0; i < num_updates; i++) {
        plugin = va_arg (plugins, int);
        update_window (plugin, state);
    }

    va_end (plugins);
}



void
update_window (int      plugin_index,
               state_t *state)
{
    switch (plugin_index) {
        case Asm:
            get_assembly_data (state);
            break;
        case Brk: 
            get_breakpoint_data (state); 
            break;
        case Dbg:
            get_debugger_output (state);
            break;
        case LcV:
            get_local_vars (state);
            break;
        case Prg:
            get_program_output (state);
            break;
        case Reg:
            get_register_data (state);
            break;
        case Src:
            get_source_path_line_memory (state);
            break;
        case Wat:
            get_watchpoint_data (state);
            break;
    }

    display_lines (state->plugins[plugin_index]->win_type,
                   state->plugins[plugin_index]->data_pos,
                   plugin_index,
                   state);
}

