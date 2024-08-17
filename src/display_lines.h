
#ifndef DISPLAY_LINES_H
#define DISPLAY_LINES_H

#include "data.h"



/*
    Display window buffer or file data in Ncurses window
    ----------
    key:   key pressed (e.g. KEY_UP, KEY_LEFT, ...) in window loop
*/

#define BUFF_TYPE  0
#define FILE_TYPE  1

void  display_lines (int type, int key, window_t *win);



/*
    Get number of rows, max cols, et al. in new window buffer string
*/
void  set_buff_rows_cols (window_t *win);



/*
    Get number of file rows, max columns, et al. in new window data file
*/
void  get_file_rows_cols (window_t *win);



#endif
