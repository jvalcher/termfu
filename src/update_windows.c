
/*
    Send window data update commands
*/
#include <stdarg.h>

#include "update_windows.h"
#include "data.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"
#include "plugins.h"
#include "utilities.h"
#include "render_window.h"

static void update_window (int, state_t*);



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



static void
update_window (int      plugin_index,
               state_t *state)
{
    int curr_debugger = state->debugger->curr;

    insert_output_start_marker (state);

    switch (plugin_index) {
    case Asm:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "x/i ", "$pc\n"); break;
        }
        break;
    case Brk:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info ", "break\n"); break;   // TODO: select breakpoint
        }
        break;
    case LcV:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info ", "locals\n"); break;
        }
        break;
    case Reg:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info ", "registers\n"); break;
        }
        break;
    case Src:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info ", "line\n"); break;
        }
        break;
    case Wat:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info ", "watchpoints\n"); break;
        }
        break;
    }

    insert_output_end_marker (state);

    parse_debugger_output (state);

#ifndef DEBUG

    if (state->plugins[plugin_index]->has_window) {
        render_window (DATA, CURRENT, plugin_index, state);
    }

#endif
}



