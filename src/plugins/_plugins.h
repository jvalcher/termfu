
#ifndef _plugins_h
#define _plugins_h

#include "../data.h"


/*
    Plugin codes
*/
extern char *plugin_code [];



/*
    Plugin function pointers
*/
typedef void (*plugin_func_t) (state_t *state);
    //
extern plugin_func_t plugin_func [];



/*
    Window data file paths
*/
extern char *win_file_path [];


#endif
