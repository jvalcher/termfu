
/*
    GDB plugin functions
*/

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "_plugin_utils.h"
#include "../data.h"




/*
    --------------
    Header plugins
    --------------
*/
int gdb_run (debug_state_t *dstate)
{
    ssize_t  bytes    = 0;
    char    *set_tty  = "-inferior-tty-set ";
    char    *gdb_run  = "-exec-run\n";

    // TOCONT: here

    // set program stdout to current tty
    char *tty_device = ttyname(STDIN_FILENO);
    bytes += write (dstate->input_pipe, set_tty,  strlen (set_tty));
    bytes += write (dstate->input_pipe, tty_device,  strlen (tty_device));
    bytes += write (dstate->input_pipe, "\n",  strlen ("\n"));

    // run program
    bytes += write (dstate->input_pipe, gdb_run, strlen (gdb_run));

    // set output file path
    dstate->out_file_path = "/tmp/termide_gdb_run.out";

    return (int)bytes;
}


int gdb_set_breakpoint (debug_state_t *dstate)
{
    ssize_t bytes       = 0;
    char *set_break_cmd = "-break-insert ",
         *break_loc     = dstate->break_point;

    bytes += write (dstate->input_pipe, set_break_cmd, strlen (set_break_cmd));
    bytes += write (dstate->input_pipe, break_loc, strlen (break_loc));
    bytes += write (dstate->input_pipe, "\n", strlen ("\n"));

    dstate->out_file_path = "/tmp/termide_gdb_set_breakpoint.out";

    return (int)bytes;
}


int gdb_next (debug_state_t *dstate)
{
    ssize_t bytes;
    char *next_cmd = "-exec-next\n";

    bytes = write (dstate->input_pipe, next_cmd, strlen (next_cmd));

    dstate->out_file_path = "/tmp/termide_gdb_next.out";

    return (int)bytes;
}


int gdb_continue (debug_state_t *dstate)
{
    ssize_t bytes;
    char *cont_cmd = "-exec-continue\n";

    bytes = write (dstate->input_pipe, cont_cmd, strlen (cont_cmd));

    dstate->out_file_path = "/tmp/termide_gdb_continue.out";

    return (int)bytes;
}


int gdb_exit (debug_state_t *dstate)
{
    ssize_t bytes;
    char *exit_cmd = "-gdb-exit\n";

    bytes = write (dstate->input_pipe, exit_cmd, strlen (exit_cmd));

    dstate->out_file_path = "/tmp/termide_gdb_exit.out";

    return (int)bytes;
}

int gdb_step (debug_state_t *dstate) 
{
    return 0;
}
int gdb_finish (debug_state_t *dstate) 
{
    return 0;
}
int gdb_print (debug_state_t *dstate) 
{
    return 0;
}
int gdb_kill (debug_state_t *dstate) 
{
    return 0;
}
int gdb_src_file (debug_state_t *dstate) 
{
    return 0;
}


/* 
    --------------
    Window plugins
    --------------
*/
int gdb_assembly (debug_state_t *dstate) 
{
    return 0;
}

int gdb_breakpoints(debug_state_t *dstate) 
{
    return 0;
}

int gdb_watches (debug_state_t *dstate) 
{
    return 0;
}

int gdb_local_vars (debug_state_t *dstate) 
{
    return 0;
}

int gdb_registers (debug_state_t *dstate) 
{
    return 0;
}

int gdb_prompt (debug_state_t *dstate) 
{
    return 0;
}

int gdb_output (debug_state_t *dstate) 
{
    return 0;
}


/*
   Non-plugin commands
*/


int gdb_get_local_vars (debug_state_t *dstate)
{
    ssize_t bytes;
    char *exit_cmd = "info locals\n";

    bytes = write (dstate->input_pipe, exit_cmd, strlen (exit_cmd));

    dstate->out_file_path = "/tmp/termide_gdb_get_local_vars.out";

    return (int)bytes;
}


/*
    Update window data
*/

