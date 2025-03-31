#ifndef PERSIST_DATA_H
#define PERSIST_DATA_H

#include "data.h"



/*
    Get watchpoints, breakpoints from PERSIST_FILE or -p <file>
*/
int get_persisted_data (state_t *state);



/*
   Save current watchpoints, breakpoints to PERSIST_FILE or -p <file>
*/
int persist_data (state_t *state);



#endif
