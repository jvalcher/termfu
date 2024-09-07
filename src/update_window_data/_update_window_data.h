
#ifndef UPDATE_WINDOW_DATA_H
#define UPDATE_WINDOW_DATA_H

#include "../data.h"

#define BEG_DATA    0      // display beginning buffer, file data
#define END_DATA    1      // display end of buffer, file data
#define LINE_DATA   2      // center data on file_data->line
#define ROW_DATA    3      // center data on current buff_data->scroll_row



/*
    Update single window's data
    ---------
    - Window enum (see plugins.h)
*/
void  update_window (int win_enum, state_t *state);



/*
    Update multiple windows' data
    ------
    - Enter <num_windows> plugin enums (see plugins.h) 
*/
void  update_windows (state_t *state, int num_windows, ...);



#endif
