
#ifndef PARSE_CONFIG_FILE_H
#define PARSE_CONFIG_FILE_H

#include "data.h"



/*
    Parse CONFIG_FILE data
    --------------
    - Allocate plugin_t structs (state->plugins)
    - Allocate window_t structs (state->plugins[i]->win)
    - Parse configuration file
    - Allocate layout_t linked list, calculate window ratios, ...
*/
int parse_config_file (state_t *state);



#endif
