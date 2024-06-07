
#ifndef RUN_PLUGIN_H
#define RUN_PLUGIN_H

#include "data.h"



/*
    Plugin function array
    ----------
    - Functions defined in plugins/...
    - Called in main.c
*/
int run_plugin (int key, 
                state_t *state);


#endif
