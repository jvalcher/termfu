
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
int gdb_assembly    (void);
int gdb_breakpoints (void);
int gdb_watches     (void);
int gdb_local_vars  (void);
int gdb_src_file    (void);
int gdb_registers   (void);
int gdb_prompt      (void);
int gdb_output      (void);
int gdb_prompt      (void);
int gdb_run         (void);
int gdb_next        (void);
int gdb_step        (void);
int gdb_continue    (void);
int gdb_finish      (void);
int gdb_print       (void);
int gdb_kill        (void);



#endif
