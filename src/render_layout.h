
#ifndef RENDER_LAYOUT_H
#define RENDER_LAYOUT_H

#include "data.h"



/*
   Render Ncurses layout <layout_title>
   ----------
   - Calculates and renders Ncurses header and windows (no data)
     based on configuration file settings set in parse_config_file()
*/
void render_layout (char *layout_title, state_t *state);



#endif
