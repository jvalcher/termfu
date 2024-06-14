
#ifndef GDB_H
#define GDB_H

#include "data.h"

void gdb_parse_output (char *in_buffer, char *debug_out_buffer, char *program_out_buffer);

int gdb_load_prog       (debug_state_t *state);
int gdb_run             (debug_state_t *state);
int gdb_set_breakpoint  (debug_state_t *state);
int gdb_next            (debug_state_t *state);
int gdb_continue        (debug_state_t *state);
int gdb_exit            (debug_state_t *state);
int gdb_get_local_vars  (debug_state_t *state);


#endif
