/*
    GDB plugins
    ---------

    Contents:

        - Header plugins
        - Window plugins
*/


#include "_plugin_utils.h"
#include "../data.h"
#include "../run_plugin.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>



/*
    Header plugins
*/
int gdb_run (layout_t *layout) 
{
    pulse_window_string ("Run", layout);
    return 1;
}
int gdb_next (layout_t *layout) 
{
    pulse_window_string ("Nxt", layout);
    return 1;
}
int gdb_step (layout_t *layout) 
{
    pulse_window_string ("Stp", layout);
    return 1;
}
int gdb_continue (layout_t *layout) 
{
    pulse_window_string ("Con", layout);
    return 1;
}
int gdb_finish (layout_t *layout) 
{
    pulse_window_string ("Fin", layout);
    return 1;
}
int gdb_print (layout_t *layout) 
{
    pulse_window_string ("Prn", layout);
    return 1;
}
int gdb_kill (layout_t *layout) 
{
    pulse_window_string ("Kil", layout);
    return 1;
}
int gdb_src_file (layout_t *layout) 
{
    pulse_window_string ("Src", layout);
    return 1;
}



/* 
    # Window plugins
*/

int gdb_assembly (layout_t *layout) 
{
    return 1;
}

int gdb_breakpoints(layout_t *layout) 
{
    return 1;
}

int gdb_watches (layout_t *layout) 
{
    return 1;
}

int gdb_local_vars (layout_t *layout) 
{
    return 1;
}

int gdb_registers (layout_t *layout) 
{
    return 1;
}

int gdb_prompt (layout_t *layout) 
{
    return 1;
}

int gdb_output (layout_t *layout) 
{
    return 1;
}



