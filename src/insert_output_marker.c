#include "insert_output_marker.h"
#include "data.h"
#include "utilities.h"



/*
    ">START"
*/
int
insert_output_start_marker (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            ret = send_command (state, "echo >START\n");
            if (ret == FAIL) {
                pfemr (ERR_DBG_CMD);
            }
            break;
        case (DEBUGGER_PDB):
            ret = send_command (state, "p \">START\"\n");
            if (ret == FAIL) {
                pfemr (ERR_DBG_CMD);
            }
            break;
    }

    return A_OK;
}


/*
    ">END"
*/
int
insert_output_end_marker (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            ret = send_command (state,"echo >END\n");
            if (ret == FAIL) {
                pfemr (ERR_DBG_CMD);
            }
            break;
        case (DEBUGGER_PDB):
            ret = send_command (state, "p \">END\"\n");
            if (ret == FAIL) {
                pfemr (ERR_DBG_CMD);
            }
            break;
    }

    return A_OK;
}



