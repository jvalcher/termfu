
/*
    GDB commands
*/

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "../data.h"

ssize_t bytes;


/*
    --------------
    Header plugins
    --------------
*/
int gdb_run (debug_state_t *dstate)
{
    char    *set_tty  = "-inferior-tty-set ";
    char    *gdb_run  = "-exec-run\n";

    // set program stdout to current tty
    char *tty_device = ttyname(STDIN_FILENO);
    write (dstate->input_pipe, set_tty,  strlen (set_tty));
    write (dstate->input_pipe, tty_device,  strlen (tty_device));
    write (dstate->input_pipe, "\n", 1);

    write (dstate->input_pipe, gdb_run, strlen (gdb_run));
    write (dstate->input_pipe, dstate->prog_path, strlen (dstate->prog_path));
    write (dstate->input_pipe, "\n", strlen ("\n"));
    return (int)bytes;
}


int gdb_set_breakpoint (debug_state_t *dstate)
{
    char *set_break_cmd = "-break-insert ";
    dstate->break_point = "main";
    bytes += write (dstate->input_pipe, set_break_cmd, strlen (set_break_cmd));
    bytes += write (dstate->input_pipe, dstate->break_point, strlen (dstate->break_point));
    bytes += write (dstate->input_pipe, "\n", 1);
    return (int)bytes;
}


int gdb_next (debug_state_t *dstate)
{
    char *next_cmd = "-exec-next\n";
    bytes = write (dstate->input_pipe, next_cmd, strlen (next_cmd));
    return (int)bytes;
}


int gdb_continue (debug_state_t *dstate)
{
    char *cont_cmd = "-exec-continue\n";
    bytes = write (dstate->input_pipe, cont_cmd, strlen (cont_cmd));
    return (int)bytes;
}


int gdb_exit (debug_state_t *dstate)
{
    char *exit_cmd = "-gdb-exit\n";
    bytes = write (dstate->input_pipe, exit_cmd, strlen (exit_cmd));
    dstate->running = false;
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
    char *info_locals_cmd = "info locals\n";
    bytes = write (dstate->input_pipe, info_locals_cmd, strlen (info_locals_cmd));
    dstate->out_file_path = "/tmp/termide_gdb_get_local_vars.out";
    return (int)bytes;
}


/*
    Update window data
*/



/*
    Parse GDB output
*/
void gdb_parse_output (debug_state_t *dstate)
{
    char line [4096];

    fprintf(dstate->out_parsed_file_ptr, "\n");

    rewind (dstate->out_file_ptr);
    while (fgets (line, sizeof(line), dstate->out_file_ptr) != NULL) {

        // program and standard GDB output
        if (line[0] != '@' && line[0] != '&' && line[0] != '^') {
            fprintf(dstate->out_parsed_file_ptr, "%s", (line + 1));
            break;
        }
    }

    // print prompt
    fprintf(dstate->out_parsed_file_ptr, "\n(gdb) ");

    fclose (dstate->out_file_ptr);
}
