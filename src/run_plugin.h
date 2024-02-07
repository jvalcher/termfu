
#ifndef RUN_PLUGIN_H
#define RUN_PLUGIN_H

#include "data.h"



/*
    Array of plugin code strings
    ----------
    - NUM_PLUGINS must be defined to allow sizeof() calls to work 
      in render_layout.c
      - Not needed if plugin_codes defined in render_layout.h/c, which 
        is more inelegant IMO
      - TODO: Make this unnecessary
*/
#define NUM_PLUGINS  19
    //
extern char *plugin_code [NUM_PLUGINS];



/*
    Plugin function array
    ----------
    - Functions defined in plugins/...
    - Called in main.c
*/
int run_plugin (int key, layout_t *layout);


#endif
