
#ifndef start_debugger_h
#define start_debugger_h

#include "data.h"



/*
    Configure, start debugger process
*/
int start_debugger (state_t *state);



/*
    Print debugger_t info to <DEBUG_OUT_FILE>
*/
void print_debugger_state (state_t *state);



#endif
