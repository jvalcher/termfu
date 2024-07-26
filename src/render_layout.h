
#ifndef RENDER_LAYOUT_H
#define RENDER_LAYOUT_H

#include "data.h"



/*
   Render Ncurses layouts
   ----------
   - Calculates and renders Ncurses header and windows (no data)
     based on configuration file settings set in parse_config_file()
   - layout_title set in CONFIG_FILE (see data.h, parse_config_file.c)
*/
void render_layout (char *layout_title, state_t *state);



#endif
