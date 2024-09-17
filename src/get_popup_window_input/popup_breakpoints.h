#ifndef POPUP_BREAKPOINTS_H
#define POPUP_BREAKPOINTS_H

#include "../data.h"



/*
    Create/delete breakpoint_t structs, update window
*/
void insert_breakpoint (state_t *state);
void delete_breakpoint (state_t *state);
void clear_all_breakpoints (state_t *state);



#endif
