
#ifndef RENDER_LAYOUT_H
#define RENDER_LAYOUT_H

#include "data.h"



/*
   Render Ncurses layout
   ----------
   - Calculates and renders Ncurses header WINDOW, data windows and subwindows (no data)
     based on configuration file settings parsed in parse_config_file()
*/
int render_layout (char *layout_title, state_t *state);



#endif
