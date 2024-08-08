#ifndef UPDATE_WINDOW_GDB_H
#define UPDATE_WINDOW_GDB_H

#include "../data.h"



void  update_assembly_gdb     (state_t *state);
void  update_breakpoints_gdb  (state_t *state);
void  update_local_vars_gdb   (state_t *state);
void  update_registers_gdb    (state_t *state);
void  update_source_file_gdb  (state_t *state);
void  update_watchpoints_gdb  (state_t *state);



#endif
