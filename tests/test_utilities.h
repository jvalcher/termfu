#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "../src/data.h"



/*
    Pause process, print PID for debugger to attach to
*/
void process_pause (void);



/*
    Create state->plugins[i]->win
    ------
    - Set the following:

        win->cols = 50;
        win->rows = 30;
        win->y = 20;
        win->x = 20;
*/
void create_ncurses_data_window (window_t *win);



#endif
