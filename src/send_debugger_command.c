
/*
    Interface for plugin and other debugger-specific functions
*/


#include "send_debugger_command.h"
#include "data.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"
#include "plugins.h"
#include "utilities.h"



void
send_debugger_command (int      plugin_index,
                       state_t *state)
{
    int curr_debugger = state->debugger->curr;

    insert_output_start_marker (state);

    switch (plugin_index) {
    case Con:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "-exec-continue\n"); break;
        }
        break;
    case Fin:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "-exec-finish\n"); break;
        }
        break;
    case Kil:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "-exec-abort\n"); break;
        }
        break;
    case Nxt:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "-exec-next\n"); break;
        }
        break;
    case Stp:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "step\n"); break;
        }
        break;
    case Run:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "-exec-run\n"); break;
        }
        break;
    case Brk:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "-break-insert ", " main\n"); break;   // TODO: breaks
        }
    case Unt:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 2, "-exec-until", "15\n"); break;       // TODO: until
        }
        break;
    case Qut:
        switch (curr_debugger) {
        case (DEBUGGER_GDB): send_command (state, 1, "-gdb-exit\n"); break;
        }
        state->debugger->running = false;
        goto skip_parse;
    }

    insert_output_end_marker (state);

    parse_debugger_output (state);

    printf ("DEBUGGER OUTPUT: \n%s\n\n", state->debugger->debugger_buffer);
    printf ("PROGRAM OUTPUT: \n%s\n\n", state->debugger->program_buffer);

    // CURRENT: Figure out how to update windows

skip_parse:
}



