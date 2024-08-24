
#include "insert_output_marker.h"
#include "data.h"
#include "utilities.h"


/*
    ">START\n"
*/
void 
insert_output_start_marker (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            send_command (state, "echo >START\n");
            break;
    }
}


/*
    ">END\n"
*/
void
insert_output_end_marker (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            send_command (state,"echo >END\n");
            break;
    }
}



