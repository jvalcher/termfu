
#ifndef PARSE_DEBUGGER_OUTPUT_H
#define PARSE_DEBUGGER_OUTPUT_H

#include "data.h"



/*
    Clean debugger output and put data inside of a buffer
    -----------
    state->debugger->data_buffer
                   ->cli_buffer
                   ->program_buffer
                   ->async_buffer
*/
int parse_debugger_output (state_t *state);



#endif
