#ifndef BREAKPOINTS_H
#define BREAKPOINTS_H

#include "../data.h"



/*
    Create, delete, clear all breakpoint_t structs, update window
*/
void insert_breakpoint (state_t *state);
void delete_breakpoint (state_t *state);
void clear_all_breakpoints (state_t *state);



#endif
