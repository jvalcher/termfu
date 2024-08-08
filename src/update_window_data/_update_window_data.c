
/*
    Send window data update commands
*/
#include <stdarg.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"

#include "update_window_gdb.h"

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
    int curr_debugger = state->debugger->curr;

    switch (plugin_index) {
    case Asm:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): update_assembly_gdb (state); break;
        }
        break;
    case Brk:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): update_breakpoints_gdb (state); break;
        }
        break;
    case LcV:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): update_local_vars_gdb (state); break;
            break;
        }
        break;
    case Reg:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): update_registers_gdb (state); break;
        }
        break;
    case Src:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): update_source_file_gdb (state); break;
        }
        break;
    case Wat:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): update_watchpoints_gdb (state); break;
        }
        break;
    }
}



