
#ifndef RENDER_WINDOW_DATA_H
#define RENDER_WINDOW_DATA_H

#include "data.h"



enum {
    HEADER_TITLE_COLOR_ON,
    HEADER_TITLE_COLOR_OFF,
    SELECT,
    DESELECT,
    DATA,
    SRC_FILE,

    CURRENT
};



void render_window (int, int, int, state_t*);



#endif
