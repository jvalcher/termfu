
#ifndef DISPLAY_LINES_H
#define DISPLAY_LINES_H

#include "data.h"

#define BUFF_TYPE  0
#define FILE_TYPE  1


/*
    Display buffer or file data in Ncurses window
    ----------
    type:  BUFF_TYPE, FILE_TYPE
    key:   key pressed (e.g. KEY_UP, KEY_LEFT, ...) in window loop
*/

int  display_lines (int type, int key, int plugin_index, state_t *state);



/*
    Get number of rows, max cols, et al. in new window buffer string
*/
void  set_buff_rows_cols (window_t *win);



/*
    Get number of file rows, max columns, et al. in new window data file
*/
int  set_file_rows_cols (window_t *win);



#endif
