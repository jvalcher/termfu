
#ifndef UPDATE_WINDOW_DATA_H
#define UPDATE_WINDOW_DATA_H

#include "../data.h"

#define BEG_DATA    0      // display beginning buffer, file data
#define END_DATA    1      // display end of buffer, file data
#define LINE_DATA   2      // center data on state->plugins[x]->win->file_data->line
#define ROW_DATA    3      // center data on current state->plugins[x]->win->buff_data->scroll_row



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
