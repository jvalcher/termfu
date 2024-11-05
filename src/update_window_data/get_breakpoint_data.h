#ifndef GET_BREAKPOINT_DATA_H
#define GET_BREAKPOINT_DATA_H

#include "../data.h"



/*
    Get breakpoint data
    -----
    state->plugins[Brk]->win->buff_data
    state->breakpoints
*/
int get_breakpoint_data (state_t *state);



#endif

