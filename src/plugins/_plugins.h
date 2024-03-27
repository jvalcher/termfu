
#ifndef _plugins_h
#define _plugins_h

#include "../data.h"

/*
    Array of plugin code strings
*/
extern char *plugin_code [];


/*
    Plugin function pointers
*/
typedef int (*plugin_func_t) (plugin_t *plugin);
    //
extern plugin_func_t plugin_func_arr [];

#endif
