#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include "../data.h"



/*
    Create, delete, clear all breakpoint_t structs, update window
*/
int insert_breakpoint (state_t *state);
int delete_breakpoint (state_t *state);
int clear_all_breakpoints (state_t *state);



#endif
