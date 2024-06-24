
#ifndef RENDER_WINDOW_DATA_H
#define RENDER_WINDOW_DATA_H

#include "data.h"

enum {
    HEADER_TITLE_COLOR_OFF,
    HEADER_TITLE_COLOR_ON,
    WINDOW_SELECT,
    WINDOW_UNSELECT,
    WINDOW_DATA,
    
    BEGINNING
};


void render_window_data (window_t*, state_t*, int, int);



#endif
