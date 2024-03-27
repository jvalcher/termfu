
/*
    GDB plugin functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "../data.h"
#include "../run_plugin.h"


void run_command (plugin_t *plugin)
{
}


/*
    --------------
    Header plugins
    --------------
*/
int gdb_run (plugin_t *plugin) 
{
    return 1;
}
int gdb_next (plugin_t *plugin) 
{
    return 1;
}
int gdb_step (plugin_t *plugin) 
{
    return 1;
}
int gdb_continue (plugin_t *plugin) 
{
    return 1;
}
int gdb_finish (plugin_t *plugin) 
{
    return 1;
}
int gdb_print (plugin_t *plugin) 
{
    return 1;
}
int gdb_kill (plugin_t *plugin) 
{
    return 1;
}
int gdb_src_file (plugin_t *plugin) 
{
    return 1;
}



/* 
    --------------
    Window plugins
    --------------
*/



int gdb_assembly (plugin_t *plugin) 
{
    return 1;
}

int gdb_breakpoints(plugin_t *plugin) 
{
    return 1;
}

int gdb_watches (plugin_t *plugin) 
{
    return 1;
}

int gdb_local_vars (plugin_t *plugin) 
{
    return 1;
}

int gdb_registers (plugin_t *plugin) 
{
    return 1;
}

int gdb_prompt (plugin_t *plugin) 
{
    return 1;
}

int gdb_output (plugin_t *plugin) 
{
    return 1;
}

