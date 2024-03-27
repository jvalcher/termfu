
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
int gdb_assembly    (plugin_t *plugin);
int gdb_breakpoints (plugin_t *plugin);
int gdb_watches     (plugin_t *plugin);
int gdb_local_vars  (plugin_t *plugin);
int gdb_src_file    (plugin_t *plugin);
int gdb_registers   (plugin_t *plugin);
int gdb_prompt      (plugin_t *plugin);
int gdb_output      (plugin_t *plugin);
int gdb_prompt      (plugin_t *plugin);
int gdb_run         (plugin_t *plugin);
int gdb_next        (plugin_t *plugin);
int gdb_step        (plugin_t *plugin);
int gdb_continue    (plugin_t *plugin);
int gdb_finish      (plugin_t *plugin);
int gdb_print       (plugin_t *plugin);
int gdb_kill        (plugin_t *plugin);



#endif
