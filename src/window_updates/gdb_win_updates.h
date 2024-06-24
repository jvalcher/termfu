
#ifndef WIN_UPDATE_GDB_H
#define WIN_UPDATE_GDB_H

#include "../data.h"

/*
   Window data updates
*/
void gdb_update_assembly    (state_t*);
void gdb_update_breakpoints (state_t*);
void gdb_update_local_vars  (state_t*);
void gdb_update_registers   (state_t*);
void gdb_update_source_file (state_t*);


#endif
