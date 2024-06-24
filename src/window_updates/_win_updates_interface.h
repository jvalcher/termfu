
#ifndef _WIN_UPDATES_INTERFACE_H
#define _WIN_UPDATES_INTERFACE_H

#include "../data.h"



void update_assembly    (state_t*);
void update_breakpoints (state_t*);
void update_local_vars  (state_t*);
void update_source_file (state_t*);
void update_registers   (state_t*);



#endif
