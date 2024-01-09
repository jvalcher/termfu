
#include "_plugin_utils.h"
#include "../data.h"
#include "../run_plugin.h"


/* 
    Header plugins
*/
int gdb_assembly (int li, layouts_t *layouts) {return 1;}
int gdb_breakpoints(int li, layouts_t *layouts) {return 1;}
int gdb_watches (int li, layouts_t *layouts) {return 1;}
int gdb_local_vars (int li, layouts_t *layouts) {return 1;}
int gdb_registers (int li, layouts_t *layouts) {return 1;}
int gdb_prompt (int li, layouts_t *layouts) {return 1;}
int gdb_output (int li, layouts_t *layouts) {return 1;}



/*
    Window plugins
*/
int gdb_run (int li, layouts_t *layouts) 
{
    pulse_window_string ("Run", li, layouts);
    return 1;
}
int gdb_next (int li, layouts_t *layouts) 
{
    pulse_window_string ("Nxt", li, layouts);
    return 1;
}
int gdb_step (int li, layouts_t *layouts) 
{
    pulse_window_string ("Stp", li, layouts);
    return 1;
}
int gdb_continue (int li, layouts_t *layouts) 
{
    pulse_window_string ("Con", li, layouts);
    return 1;
}
int gdb_finish (int li, layouts_t *layouts) 
{
    pulse_window_string ("Fin", li, layouts);
    return 1;
}
int gdb_print (int li, layouts_t *layouts) 
{
    pulse_window_string ("Prn", li, layouts);
    return 1;
}
int gdb_kill (int li, layouts_t *layouts) 
{
    pulse_window_string ("Kil", li, layouts);
    return 1;
}
int gdb_src_file (int li, layouts_t *layouts) 
{
    pulse_window_string ("Src", li, layouts);
    return 1;
}

