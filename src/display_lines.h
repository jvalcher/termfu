
#ifndef DISPLAY_LINES_H
#define DISPLAY_LINES_H

#include "data.h"


/*
    Display buffer or file data in plugin's ncurses data subwindow
    ----------
    - <key>:
        - Data positions:
            BEG_DATA    - display from start
            END_DATA    - display last lines
            ROW_DATA    - center on row
        - Keys: 
            KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
            KEY_PPAGE, KEY_NPAGE, KEY_HOME, KEY_END
*/
int  display_lines (int key, int plugin_index, state_t *state);


#endif
