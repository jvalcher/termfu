
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
int gdb_assembly    (int li, layouts_t *layouts);
int gdb_breakpoints (int li, layouts_t *layouts);
int gdb_watches     (int li, layouts_t *layouts);
int gdb_local_vars  (int li, layouts_t *layouts);
int gdb_src_file    (int li, layouts_t *layouts);
int gdb_registers   (int li, layouts_t *layouts);
int gdb_prompt      (int li, layouts_t *layouts);
int gdb_output      (int li, layouts_t *layouts);
int gdb_prompt      (int li, layouts_t *layouts);
int gdb_run         (int li, layouts_t *layouts);
int gdb_next        (int li, layouts_t *layouts);
int gdb_step        (int li, layouts_t *layouts);
int gdb_continue    (int li, layouts_t *layouts);
int gdb_finish      (int li, layouts_t *layouts);
int gdb_print       (int li, layouts_t *layouts);
int gdb_kill        (int li, layouts_t *layouts);



#endif
