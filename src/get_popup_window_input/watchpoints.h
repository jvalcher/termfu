#ifndef WATCHPOINTS_H
#define WATCHPOINTS_H

#include "../data.h"



/*
   Insert, delete, clear all watchpoints
*/
int insert_watchpoint (state_t *state);
int delete_watchpoint (state_t *state);
int clear_all_watchpoints (state_t *state);



#endif

