
#ifndef UPDATE_WINDOW_DATA_H
#define UPDATE_WINDOW_DATA_H

#include "../data.h"

#define NULL_PLUGIN  -5


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
    Usage:
        update_windows (Asm, Brk, Src);
*/
int  update_windows_impl (int num_windows, ...);
#define update_windows(...)  update_windows_impl (100, __VA_ARGS__, NULL_PLUGIN)



#endif
