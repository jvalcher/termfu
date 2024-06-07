#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>

#include "data.h"
#include "utils.h"

char *flush_cmd  = "call fflush (0)\n";
char *set_tty = "-inferior-tty-set /dev/pts/6\n";


/*
    Codes
    -----

termide_back,       "Bak"
termide_builds,     "Bld"
termide_layouts,    "Lay"

gdb_assembly,       "Asm"
gdb_breakpoints,    "Brk"
gdb_continue,       "Con"
gdb_finish,         "Fin"
gdb_kill,           "Kil"
gdb_local_vars,     "LcV"
gdb_next,           "Nxt"
gdb_output,         "Out"
gdb_prompt,         "Prm"
gdb_print,          "Prn"
gdb_registers,      "Reg"
gdb_run,            "Run"
gdb_src_file,       "Src"
gdb_step,           "Stp"
gdb_watches         "Wat"

    TODO:
    ----

    - Add mutex that sets when writing to pipe and doesn't unlock until (gdb) prompt seen in output process

    -interpreter-exec  for typed commands
*/


int gdb_run (debug_state_t *state)
{
    ssize_t  bytes    = 0;
    char    *set_tty  = "-inferior-tty-set ";
    char    *gdb_run  = "-exec-run\n";

    // set program stdout to current tty
    char *tty_device = ttyname(STDIN_FILENO);
    bytes += write (state->input_pipe, set_tty,  strlen (set_tty));
    bytes += write (state->input_pipe, tty_device,  strlen (tty_device));
    bytes += write (state->input_pipe, "\n",  strlen ("\n"));

    // run program
    bytes += write (state->input_pipe, gdb_run, strlen (gdb_run));

    return (int)bytes;
}


int gdb_set_breakpoint (debug_state_t *state)
{
    ssize_t bytes       = 0;
    char *set_break_cmd = "-break-insert ",
         *break_loc     = state->break_point;

    // set breakpoint
    bytes += write (state->input_pipe, set_break_cmd, strlen (set_break_cmd));
    bytes += write (state->input_pipe, break_loc, strlen (break_loc));
    bytes += write (state->input_pipe, "\n", strlen ("\n"));

    return (int)bytes;
}


int gdb_next (debug_state_t *state)
{
    ssize_t bytes;
    char *next_cmd = "-exec-next\n";

    bytes = write (state->input_pipe, next_cmd, strlen (next_cmd));

    return (int)bytes;
}


int gdb_continue (debug_state_t *state)
{
    ssize_t bytes;
    char *cont_cmd = "-exec-continue\n";

    bytes = write (state->input_pipe, cont_cmd, strlen (cont_cmd));

    return (int)bytes;
}


int gdb_exit (debug_state_t *state)
{
    ssize_t bytes;
    char *exit_cmd = "-gdb-exit\n";

    bytes = write (state->input_pipe, exit_cmd, strlen (exit_cmd));

    return (int)bytes;
}


/*
   Non-plugin commands
*/


int gdb_get_local_vars (debug_state_t *state)
{
    ssize_t bytes;
    char *exit_cmd = "info locals\n";

    bytes = write (state->input_pipe, exit_cmd, strlen (exit_cmd));

    return (int)bytes;
}


/*
    Update window data
*/

