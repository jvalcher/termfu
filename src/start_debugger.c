#include "data.h"
#include "gdb.h"

#define DEBUGGER_UNKNOWN   0
#define DEBUGGER_GDB       1

#define PIPE_READ            0
#define PIPE_WRITE           1

#define PIPE_FAILED         -1
#define FORK_FAILED         -2
#define DEBUG_LAUNCH_FAILED -3
#define EXIT_PROGRAM        -5


void start_debugger_process (debug_state_t *dstate)
void start_debugger_reader (debug_state_t *dstate);

int    debug_in_pipe  [2],
       debug_out_pipe [2];
pid_t  debugger_pid,
       debug_out_pid,
       ret_pid;

char *gdb_cmd[]      = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};
char *debugger_quit  = "^exit";



/*
    Start debugger
*/
void start_debugger (debug_state_t *dstate)
{
    enum Debugger debugger;

    // create debugger pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) 
    {
        perror("Debugger pipe");
        exit (PIPE_FAILED);
    }
    dstate->input_pipe  = debug_in_pipe [PIPE_WRITE];
    dstate->output_pipe = debug_out_pipe [PIPE_READ];

    // TODO: Get file type, set debugger
    dstate->debugger = GDB;

    start_debugger_process (dstate);
    start_debugger_reader (dstate);
}



/*
    Start debugger process
*/
void start_debugger_process (debug_state_t *dstate)
{
    char  **cmd;

    // set command
    switch (dstate->debugger) {

        // GDB
        case (GDB):
            cmd = gdb_cmd;
            cmd [3] = dstate->prog_path;
            break;
    }

    // create debugger process
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        perror ("Debugger fork");
        exit (FORK_FAILED);
    }

    if (debugger_pid == 0) {

        dup2  (debug_in_pipe [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (cmd[0], cmd);

        perror ("Debugger process");
        exit (DEBUG_LAUNCH_FAILED);
    }

}



/*
    Start debugger reading process
*/
void start_debugger_reader (debug_state_t *dstate)
{
    char    output_buffer [256],
            debug_out_buffer [256],
            program_out_buffer [256];
    size_t  bytes_read;
    int     i, j;
    FILE   *out_file_ptr;
    bool    running,
            is_newline,
            is_output;

    debug_out_pid = fork ();
    if (debug_out_pid == -1) {
        perror ("Debugger output fork");
        exit (FORK_FAILED);
    }

    // debugger output process
    if (debug_out_pid == 0) {

        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_WRITE]);

        //out_file_ptr = fopen("debug_out.log", "w");

        // read pipe output
        running = true;
        while (running) 
        {
            bytes_read = read (debug_out_pipe [PIPE_READ], 
                               output_buffer, 
                               sizeof (output_buffer) - 1);
            output_buffer [bytes_read] = '\0';

            //printf ("%d\n", bytes_read);
            //printf ("%s", output_buffer);

            memset (debug_out_buffer, '\0', bytes_read);
            memset (program_out_buffer, '\0', bytes_read);
            gdb_parse_output (output_buffer, debug_out_buffer, program_out_buffer);
            printf ("%s", debug_out_buffer);
            printf ("%s", program_out_buffer);
            fprintf (

            // exit
            if (strstr (output_buffer, dstate->exit_str) != NULL) {
                running = false;
            }
        }

        //fclose (out_file_ptr);
        exit (0);
    }

}



/*
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
*/
