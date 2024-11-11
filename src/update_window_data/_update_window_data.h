
#ifndef UPDATE_WINDOW_DATA_H
#define UPDATE_WINDOW_DATA_H

#include "../data.h"



/*
    Update ncurses window data (thread)
*/
void *update_window_thread (void *statev);



/*
    Update single window's data
*/
int  update_window (int plugin_index);



/*
    Update multiple windows' data
    ------
    num_windows == number of plugin index arguments
*/
int  update_windows (int num_windows, ...);



#endif
