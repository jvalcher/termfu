
#ifndef DISPLAY_LINES_H
#define DISPLAY_LINES_H

#include "data.h"



/*
    Display buffer or file data in Ncurses window
    ----------
    - Variables: 
        key: BEG_DATA, END_DATA, ROW_DATA,
             KEY_UP, KEY_DOWN, KEY_PPAGE, KEY_NPAGE, KEY_HOME, KEY_END
    - Returns:
        - A_OK, FAIL
*/
int  display_lines (int key, int plugin_index, state_t *state);



/*
    Display buffer lines
*/
int display_scroll_buff_lines (int key, int plugin_index, state_t *state);



#endif
