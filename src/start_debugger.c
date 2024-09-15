#include <stddef.h>
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

static void   start_debugger_proc         (state_t*);



void
start_debugger (state_t *state)
{
    start_debugger_proc (state);

    insert_output_end_marker (state);

    parse_debugger_output (state);
}



static void
start_debugger_proc (state_t *state)
{
    pid_t   debugger_pid;
    int     debug_in_pipe  [2],
            debug_out_pipe [2];

    debugger_t *debugger = state->debugger;
    
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1)
    {
        pfeme ("Debugger pipe creation failed\n");
    }
    debugger->stdin_pipe  = debug_in_pipe [PIPE_WRITE];
    debugger->stdout_pipe = debug_out_pipe [PIPE_READ];

    // fork
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        pfeme ("Debugger fork failed\n");
    }

    // start debugger
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe  [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);

        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        /*
        dev_null = open ("/dev/null", O_WRONLY);
        dup2  (dev_null, STDERR_FILENO);
        close (dev_null);
        */

        execvp (state->command[0], state->command);

        pfeme ("Debugger start failed\n");
    }

    if (debugger_pid > 0) {
    
        debugger->running = true;

        close (debug_in_pipe   [PIPE_READ]);
        close (debug_out_pipe  [PIPE_WRITE]);
    }
}



