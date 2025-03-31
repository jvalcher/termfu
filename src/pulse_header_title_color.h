
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
   - Switches header title string characters' color "on" and "off"
     to indicate usage
*/
int pulse_header_title_color (int plugin_index, state_t *state, int pulse_state);



#endif
