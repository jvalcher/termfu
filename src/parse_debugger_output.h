
#ifndef PARSE_DEBUGGER_OUTPUT_H
#define PARSE_DEBUGGER_OUTPUT_H

#include "data.h"



/*
    Run initial parse of debugger command output
*/
void parse_debugger_output (state_t *state);



/*
    Parse specific debugger output (for testing)
*/
void parse_debugger_output_gdb (debugger_t *debugger);
void parse_debugger_output_pdb (debugger_t *debugger);



#endif
