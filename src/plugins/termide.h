
#ifndef TERMIDE_H
#define TERMIDE_H

#include "../data.h"

/*
termIDE
-------

    empty_func          plugin_function[] placeholder, not called

    termide_back        Bak     Back  (i.e. unfocus window, quit termIDE)
    termide_layouts     Lay     Display layouts
    termide_builds      Bld     Display builds
*/
int empty_func      (debug_state_t *dstate);
    //
int termide_back    (debug_state_t *dstate);
int termide_layouts (debug_state_t *dstate);
int termide_builds  (debug_state_t *dstate);



#endif
