#include "insert_output_marker.h"
#include "data.h"
#include "utilities.h"


/*
    ">START"
*/
int
insert_output_start_marker (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (send_command (state, "echo >START\n") == RET_FAIL) {
                pfemr ("Failed to send GDB start marker");
            }
            break;
        case (DEBUGGER_PDB):
            if (send_command (state, "p \">START\"\n") == RET_FAIL) {
                pfemr ("Failed to send PDB start marker");
            }
            break;
    }

    return RET_OK;
}


/*
    ">END"
*/
int
insert_output_end_marker (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (send_command (state,"echo >END\n") == RET_FAIL) {
                pfemr ("Failed to send GDB end marker");
            }
            break;
        case (DEBUGGER_PDB):
            if (send_command (state, "p \">END\"\n") == RET_FAIL) {
                pfemr ("Failed to send PDB end marker");
            }
            break;
    }

    return RET_OK;
}



