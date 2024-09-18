#ifndef WATCHPOINTS_H
#define WATCHPOINTS_H

#include "../data.h"



/*
   Insert, delete, clear all watchpoints
*/
void insert_watchpoint (state_t *state);
void delete_watchpoint (state_t *state);
void clear_all_watchpoints (state_t *state);



#endif

