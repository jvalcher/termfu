
#ifndef PULSE_HEADER_TITLE_COLOR_H
#define PULSE_HEADER_TITLE_COLOR_H

#include "data.h"


// pulse states
enum {
    ON,
    OFF,
};



/*
   Pulse header title color on/off
   -------
   - Finds and switches header title strings on and off to indicate usage
*/
void pulse_header_title_color (int plugin_index, state_t *state, int pulse_state);



#endif
