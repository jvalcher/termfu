
#ifndef UPDATE_WINDOW_DATA_H
#define UPDATE_WINDOW_DATA_H

#include "../data.h"



/*
    Update single window's data
*/
int  update_window (int plugin_index, state_t *state);



/*
    Update multiple windows' data
    ------
    num_windows == number of plugin index arguments
*/
int  update_windows (state_t *state, int num_windows, ...);



#endif
