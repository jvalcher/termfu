
/*
    Render Ncurses window
    --------------------

*/


#ifndef render_screen_h
#define render_screen_h


#include "parse_config.h"

#define MAX_TITLE_LEN       20
#define MAX_Y_SEGMENTS      10      // per layout
#define MAX_X_SEGMENTS      10      // per layout

extern char *win_codes[];


// Return index used in window_data struct 
// from window symbol character (A-Z, a-z)
//
//     char  ascii  index
//     ----  -----  -----
//     'A'    65      0
//     'Z'    90     25
//     'a'    97     26
//     'z'    122    51
//
//      get_win_index ('B') --> 1
//
//int get_win_index (char symbol);


// Render main window
//
//   i       - layouts_t index
//   layouts - layouts_t struct
// 
void render_screen (int i, layouts_t *layouts);


#endif
