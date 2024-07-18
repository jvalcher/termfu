#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "start_debugger.h"
#include "data.h"
#include "parse_debugger_output.h"
#include "utilities.h"
#include "insert_output_marker.h"

static void   configure_debugger          (debugger_t*);
static void   start_debugger_proc         (debugger_t*);

// TODO: 
//  - get this via plugins/_interface
//  - add more gdb flags
# define GDB_PROG_INDEX  3
char *gdb_cmd[] = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};



void
start_debugger (state_t *state)
{
    configure_debugger (state->debugger);

    start_debugger_proc (state->debugger);

    insert_output_end_marker (state);

    parse_debugger_output (state);
}



static void
configure_debugger (debugger_t *debugger)
{
    // TODO: Get file type -> set debugger
    debugger->curr = DEBUGGER_GDB;
}



/*
    Start debugger process
*/
static void
start_debugger_proc (debugger_t *debugger)
{
    char  **cmd;
    pid_t  debugger_pid;
    int  debug_in_pipe  [2],
         debug_out_pipe [2];
    
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1)
    {
        pfeme ("Debugger pipe creation failed");
    }
    debugger->stdin_pipe      = debug_in_pipe [PIPE_WRITE];
    debugger->stdout_pipe     = debug_out_pipe [PIPE_READ];

    // set command
    switch (debugger->curr) {
        case (DEBUGGER_GDB):
            cmd = gdb_cmd;      // TODO: see globals
            cmd [GDB_PROG_INDEX] = debugger->prog_path;
            break;
    }

    // fork
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        perror ("Debugger fork");
        exit (EXIT_FAILURE);
    }

    // start debugger
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe  [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (cmd[0], cmd);

        pfeme ("Debugger start failed");
    }

    if (debugger_pid > 0) {
    
        debugger->running = true;

        close (debug_in_pipe   [PIPE_READ]);
        close (debug_out_pipe  [PIPE_WRITE]);
    }
}



