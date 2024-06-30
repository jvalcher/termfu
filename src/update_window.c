
/*
    Send window data update commands
*/

#include "update_window.h"
#include "data.h"
#include "insert_output_marker.h"
#include "plugins.h"
#include "utilities.h"


void
update_window (int      plugin_index,
               state_t *state)
{
    int curr_debugger = state->debugger->curr;

    insert_output_start_marker (state);

    switch (plugin_index) {
    case Asm:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "x/i", "$pc"); break;
        }
        break;
    case Brk:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "break", "main"); break;   // TODO: select breakpoint
        }
        break;
    case LcV:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info", "locals"); break;
        }
        break;
    case Reg:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info", "registers"); break;
        }
        break;
    case Src:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "list"); break;
        }
        break;
    case Wat:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "info", "watchpoints"); break;
        }
        break;
    }

    insert_output_end_marker (state);
}



