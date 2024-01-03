
#ifndef RUN_PLUGIN_H
#define RUN_PLUGIN_H

#include "data.h"

/*
    Plugin function array
    ----------
    - Functions defined in plugins/...
    - Called in main.c
*/
    //
typedef int (*plugin_func_t) (void);
    //
extern plugin_func_t  plugin [];


#endif
