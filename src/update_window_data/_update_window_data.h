
#ifndef UPDATE_WINDOW_H
#define UPDATE_WINDOW_H

#include "../data.h"

#define NEW_WIN    0      // display lines of new buffer, file




/*
    Update multiple windows' data
    ------
    - Enter <num_windows> plugin enums (see plugins.h) 
*/
void  update_windows (state_t *state, int num_windows, ...);



/*
    Update single window's data
    ---------
    - Window enum (see plugins.h)
*/
void  update_window (int win_enum, state_t *state);






#endif
