
#ifndef PARSE_CONFIG_FILE_H
#define PARSE_CONFIG_FILE_H

#include "data.h"



/*
    Parse CONFIG_FILE data
    --------------
    - Allocate and add plugins to `plugin` *plugin_t array  (plugins.h)
    - Allocate layout_t linked list, calculate window ratios, ...
    - ...
*/
void parse_config_file (state_t *state);



#endif
