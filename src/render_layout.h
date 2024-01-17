
/*
    Render Ncurses window
    --------------------

*/


#ifndef render_screen_h
#define render_screen_h


#include "parse_config.h"


extern char *plugin_code [];

// Render main window
//
//   layout - layouts_t struct
// 
void render_layout (layout_t *layout);


#endif
