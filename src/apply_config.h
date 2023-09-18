
/*
    Apply configuration file
*/

#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H

#include "apply_config_layout.h"


// Parse config_file_data struct (get_config.h)
// Pass resulting data into configuration structs (apply_config_*.h)
void apply_config (struct layouts *layouts);


#endif
