
#ifndef RENDER_WINDOW_DATA_H
#define RENDER_WINDOW_DATA_H

#include "data.h"



enum {
    SELECT,
    DESELECT,
    DATA,
};



/*
    TODO: Separate into separate files, functions
*/
void render_window (int, int, int, state_t*);



#endif
