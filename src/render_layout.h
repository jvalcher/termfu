
#ifndef RENDER_LAYOUT_H
#define RENDER_LAYOUT_H

#include "data.h"



/*
   Render Ncurses layouts
   ----------
   Calculates and renders Ncurses header and windows (no data)
   based on configuration file settings set in parse_config_file()
*/
void render_layout (char*, state_t*);



#endif
