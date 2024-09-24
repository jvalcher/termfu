#ifndef GET_BINARY_PATH_H
#define GET_BINARY_PATH_H

#include <time.h>
#include "../data.h"



/*
    Get path and last update time of program binary
    ----------

    state->debugger->prog_path
    state->debugger->prog_update_time
*/
int get_binary_path_time (state_t *state);



#endif
