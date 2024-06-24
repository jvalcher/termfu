
/*
    Send window data update commands
*/

#include "_win_updates_interface.h"
#include "gdb_win_updates.h"
#include "_insert_output_marker.h"

static char *get_code_path (char*, plugin_t*);

char *update_path;



void update_assembly (state_t *state)
{
    insert_output_start_marker ("Asm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_update_assembly (state); break;
    }
    insert_output_end_marker (state);
}



void update_breakpoints (state_t *state)
{
    insert_output_start_marker ("Brk", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_update_breakpoints (state); break;
    }
    insert_output_end_marker (state);
}



void update_local_vars (state_t *state)
{
    insert_output_start_marker ("LcV", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_update_local_vars (state); break;
    }
    insert_output_end_marker (state);
}



void update_source_file (state_t *state)
{
    insert_output_start_marker ("Src", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_update_source_file (state); break;
    }
    insert_output_end_marker (state);
}



void update_registers (state_t *state)
{
    insert_output_start_marker ("Reg", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_update_local_vars (state); break;
    }
    insert_output_end_marker (state);
}



