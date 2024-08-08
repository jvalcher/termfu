
#include "insert_output_marker.h"
#include "data.h"
#include "utilities.h"
#include "parse_debugger_output.h"


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



/*
    ">EXIT\n"
*/
void
insert_output_exit_marker (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            send_command (state, "echo >EXIT\n");
            break;
    }
}



