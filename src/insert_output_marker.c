#include "insert_output_marker.h"
#include "data.h"
#include "utilities.h"


/*
    ">START"
*/
void 
insert_output_start_marker (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB): send_command (state, "echo >START\n"); break;
        case (DEBUGGER_PDB): send_command (state, "p \">START\"\n"); break;
    }
}


/*
    ">END"
*/
void
insert_output_end_marker (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB): send_command (state,"echo >END\n"); break;
        case (DEBUGGER_PDB): send_command (state, "p \">END\"\n"); break;
    }
}



