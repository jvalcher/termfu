
#ifndef PARSE_DEBUGGER_OUTPUT_H
#define PARSE_DEBUGGER_OUTPUT_H

#include "data.h"



/*
    Run initial parse of debugger command output
*/
void parse_debugger_output (state_t *state);



/*
    Copy character into debugger buffer
    ---------
    state->debugger-><buffer>
    buff_index:  one of the below enum indexes
*/
enum { FORMAT_BUF, DATA_BUF, CLI_BUF, PROGRAM_BUF, ASYNC_BUF };

void cp_dchar (debugger_t *debugger, char ch, int buff_index);



/*
    Parse specific debugger output (for testing)
*/
void parse_debugger_output_gdb (debugger_t *debugger);
void parse_debugger_output_pdb (debugger_t *debugger);



#endif
