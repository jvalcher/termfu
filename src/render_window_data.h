
#ifndef RENDER_WINDOW_DATA_H
#define RENDER_WINDOW_DATA_H

#include "data.h"

enum {
    HEADER_TITLE_COLOR_ON,
    HEADER_TITLE_COLOR_OFF,
    SELECT_WINDOW_COLOR,
    DESELECT_WINDOW_COLOR,
    RENDER_DATA,
    BEGINNING
};



void render_window_data (int, state_t*, int, int);



#endif
