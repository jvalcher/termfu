
/*
    GDB commands
*/

#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#include "gdb_plugins.h"
#include "_plugins_interface.h"
#include "../data.h"
#include "../utilities.h"


ssize_t pbytes;
int ch;



/*****************
  Debugger output
 *****************/


/*
    --------------
    Header plugins
    --------------
*/
void gdb_run (state_t *state)
{
    const char *run_cmd   = "-exec-run\n";
    pbytes += write (INPUT_PIPE, run_cmd, strlen (run_cmd));
    CHECK_BYTES(pbytes);
}


void gdb_pwin_set_breakpoint (state_t *state)
{
    const char *break_cmd = "-break-insert ";

    // TODO: 
        // popup window to enter breakpoint line number, string
    const char *breakpoint = "main";

    pbytes  = write (INPUT_PIPE, break_cmd, strlen (break_cmd));
    pbytes += write (INPUT_PIPE, breakpoint, strlen (breakpoint));
    pbytes += write (INPUT_PIPE, "\n", 1);
    CHECK_BYTES(pbytes);
}


void gdb_next (state_t *state)
{
    const char *next_cmd = "-exec-next\n";
    pbytes = write (INPUT_PIPE, next_cmd, strlen (next_cmd));
    CHECK_BYTES(pbytes);
}


void gdb_step (state_t *state) 
{
    const char *step_cmd = "-exec-step\n";
    pbytes = write (INPUT_PIPE, step_cmd, strlen (step_cmd));
    CHECK_BYTES(pbytes);
}


void gdb_continue (state_t *state)
{
    const char *cont_cmd = "-exec-continue\n";
    pbytes = write (INPUT_PIPE, cont_cmd, strlen (cont_cmd));
    CHECK_BYTES(pbytes);
}


void gdb_finish (state_t *state) 
{
    const char *fin_cmd = "-exec-finish\n";
    pbytes = write (INPUT_PIPE, fin_cmd, strlen (fin_cmd));
    CHECK_BYTES(pbytes);
}



void gdb_kill (state_t *state)
{
    const char *kill_cmd = "-exec-abort\n";
    pbytes = write (INPUT_PIPE, kill_cmd, strlen (kill_cmd));
    CHECK_BYTES(pbytes);
}



void gdb_exit (state_t *state)
{
    const char *dbg_quit = "-gdb-exit\n";
    const char *rdr_quit = "echo >EXIT\n";

    // exit debugger reader process
    pbytes += write (INPUT_PIPE, rdr_quit, strlen (rdr_quit));

    // exit debugger process
    pbytes = write (INPUT_PIPE, dbg_quit, strlen (dbg_quit));

    CHECK_BYTES(pbytes);
}



/****************
  Window plugins
 ****************/


void gdb_win_assembly (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_prog_output (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_breakpoints(state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_watches (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_local_vars (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_prompt (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_src_file (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



void gdb_win_registers (state_t *state)
{
    while ((ch = getchar()) != state->win_keys->back) {
        run_other_win_key (ch, state);
    }
}



