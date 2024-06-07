#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "data.h"
#include "utilities.h"

char *gdb_cmd []       = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};
char *gdb_exit_str     = "^exit";
char *gdb_out_done_str = "(gdb)";


void start_debugger (debug_state_t *dstate)
{
    char   **cmd;
    int      debug_in_pipe  [2],
             debug_out_pipe [2];
    pid_t    debugger_pid;

    // create debugger I/O pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) {
        pfeme ("Debugger pipe error\n");
    }

    // set debugger state
    dstate->input_pipe = debug_in_pipe [PIPE_WRITE];
    dstate->output_pipe = debug_out_pipe [PIPE_READ];
        //
    switch (dstate->debugger) {

        // GDB
        case (DEBUGGER_GDB):
            cmd = gdb_cmd;
            cmd [3] = dstate->prog_path;
            dstate->out_done_str = gdb_out_done_str;
            dstate->exit_str = gdb_exit_str;
            break;
    }

    // create debugger process
    debugger_pid = fork ();
    if (debugger_pid  == -1) {
        pfeme ("Debugger fork error\n");
    }

    // start debugger process
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (cmd[0], cmd);

        pfeme ("Debugger process error\n");
    }
}

