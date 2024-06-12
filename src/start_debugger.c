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
#include "run_plugin.h"
#include "plugins/gdb.h"


// Debugger strings

char *debug_out_path        = "/tmp/termide_debugger.out";
char *debug_parsed_out_path = "/tmp/termide_debugger_parsed.out";

    // GDB
char *gdb_cmd []          = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};



/*
    Start debugger process
*/
void start_debugger (debug_state_t *dstate)
{
    char  **cmd;
    pid_t   debugger_pid;
    int     debug_in_pipe  [2],
            debug_out_pipe [2];
    FILE   *out_file_ptr;

    // create debugger I/O pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) {
        pfeme ("Debugger pipe error\n");
    }

    // set debugger pipes
    dstate->input_pipe  = debug_in_pipe [PIPE_WRITE];
    dstate->output_pipe = debug_out_pipe [PIPE_READ];

    // set debugger command, output done string, exit string 
    switch (dstate->debugger) {

        // GDB
        case (DEBUGGER_GDB):
            cmd = gdb_cmd;
            cmd [3] = dstate->prog_path;
            break;
    }

    // create debugger process
    debugger_pid = fork ();
    if (debugger_pid  == -1) {
        pfeme ("Debugger fork error\n");
    }

    // debugger process
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        // start debugger
        execvp (cmd[0], cmd);

        pfeme ("Debugger process error\n");
    }

    // parent process
    if (debugger_pid > 0) {
        
        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_WRITE]);

        dstate->running = true;
        dstate->debugger_pid = debugger_pid;
        dstate->out_file_path = debug_out_path; 

        // erase parsed output file, open for appending
        dstate->out_parsed_file_path = debug_parsed_out_path;
        dstate->out_parsed_file_ptr = fopen (dstate->out_parsed_file_path, "w");
        fclose (dstate->out_parsed_file_ptr);
        dstate->out_parsed_file_ptr = fopen (dstate->out_parsed_file_path, "a");
    }
}

