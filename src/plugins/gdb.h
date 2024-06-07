
#ifndef GDB_H
#define GDB_H

#include "../data.h"

/*

------
GDB Plugin
------

      Function         Code
     ----------       ------

    - Window-related actions
        - e.g. focus, scroll, search, list, add, delete, ...

    gdb_assembly        Asm     Assembly code
    gdb_breakpoints     Brk     Breakpoints
    gdb_watches         Wat     Watched variables
    gdb_local_vars      LcV     Local variables
    gdb_src_file        Src     Source file(s)
    gdb_registers       Reg     Registers
    gdb_prompt          Prn     Print values
    gdb_output          Out     Output
    gdb_prompt          Prm     Command prompt

    - Navigation

    gdb_run             Run     (Re)run program
    gdb_next            Nxt     Next
    gdb_step            Stp     Step
    gdb_continue        Con     Continue
    gdb_finish          Fin     Finish
    gdb_print           Prn     Print value
    gdb_kill            Kil     Kill

*/
int gdb_assembly    (debug_state_t *dstate);
int gdb_breakpoints (debug_state_t *dstate);
int gdb_watches     (debug_state_t *dstate);
int gdb_local_vars  (debug_state_t *dstate);
int gdb_src_file    (debug_state_t *dstate);
int gdb_registers   (debug_state_t *dstate);
int gdb_prompt      (debug_state_t *dstate);
int gdb_output      (debug_state_t *dstate);
int gdb_prompt      (debug_state_t *dstate);
int gdb_run         (debug_state_t *dstate);
int gdb_next        (debug_state_t *dstate);
int gdb_step        (debug_state_t *dstate);
int gdb_continue    (debug_state_t *dstate);
int gdb_finish      (debug_state_t *dstate);
int gdb_print       (debug_state_t *dstate);
int gdb_kill        (debug_state_t *dstate);



#endif
