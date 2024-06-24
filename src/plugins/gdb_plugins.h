
#ifndef GDB_H
#define GDB_H

/************************
  GDB debugger functions
 ************************/

#include "../data.h"


/*
   Non-plugin
*/
void  gdb_parse_output               (int*, char*, char*, char*, char*);
void  gdb_insert_output_start_marker (char*, state_t*);
void  gdb_insert_output_end_marker   (char*, state_t*);


/*
    Header
*/
void  gdb_run               (state_t*);
void  gdb_next              (state_t*);
void  gdb_step              (state_t*);
void  gdb_continue          (state_t*);
void  gdb_finish            (state_t*);
void  gdb_kill              (state_t*);
void  gdb_exit              (state_t*);


/*
    Windows
*/
void  gdb_win_assembly      (state_t*);
void  gdb_win_breakpoints   (state_t*);
void  gdb_win_watches       (state_t*);
void  gdb_win_local_vars    (state_t*);
void  gdb_win_src_file      (state_t*);
void  gdb_win_registers     (state_t*);
void  gdb_win_prompt        (state_t*);
void  gdb_win_prog_output   (state_t*);


/*
    Popup windows
*/
void  gdb_pwin_set_break    (state_t*);


/*
   Window data updates
*/
void gdb_update_assembly    (state_t*);
void gdb_update_breakpoints (state_t*);
void gdb_update_local_vars  (state_t*);
void gdb_update_registers   (state_t*);
void gdb_update_source_file (state_t*);


#endif
