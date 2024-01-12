
#ifndef GDB_H
#define GDB_H


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
int gdb_assembly    (layout_t *layout);
int gdb_breakpoints (layout_t *layout);
int gdb_watches     (layout_t *layout);
int gdb_local_vars  (layout_t *layout);
int gdb_src_file    (layout_t *layout);
int gdb_registers   (layout_t *layout);
int gdb_prompt      (layout_t *layout);
int gdb_output      (layout_t *layout);
int gdb_prompt      (layout_t *layout);
int gdb_run         (layout_t *layout);
int gdb_next        (layout_t *layout);
int gdb_step        (layout_t *layout);
int gdb_continue    (layout_t *layout);
int gdb_finish      (layout_t *layout);
int gdb_print       (layout_t *layout);
int gdb_kill        (layout_t *layout);



#endif
