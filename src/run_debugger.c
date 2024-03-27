#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/wait.h>

#include "data.h"
#include "utilities.h"

#define PIPE_READ   0
#define PIPE_WRITE  1

// debugger commands, quit strings
//
    // gdb
int   gdb_prog_index  = 3;
char *gdb_start_cmd[] = {"gdb", "--quiet", "--interpreter=mi", NULL};
char *gdb_quit_str    = "^exit";

    // ...



void set_debugger (debug_state_t *state, char *file_name);



/*
    Start debugger processes
*/
void run_debugger (layout_t *layout, 
                   char     *prog_name)
{
    int    debug_in_pipe  [2],
           debug_out_pipe [2];
    pid_t  debugger_pid,
           debug_out_pid,
           ret_pid;

    char         **debug_start_cmd;
    char          *debug_quit_str,
                  *quit_string;
    char           debug_out_buffer [256],
                   src_path [256];
    int            status,
                   exit_status;
    size_t         bytes_read;
    debug_state_t *state;

    // create debugger state  (layout->debug_state)
    // -----------
    // - used by plugin functions
    //
        // allocate, add to layout
    layout->debug_state = malloc (sizeof (debug_state_t));
    state = layout->debug_state;

        // set debugger
    set_debugger (state, prog_name);

        // set input, output pipes
    state->input_pipe = debug_in_pipe [PIPE_WRITE];
    state->output_pipe = debug_out_pipe [PIPE_READ];

        // set debugger command, quit string
    switch (state->debugger) {

        // gdb
        case DEBUGGER_GDB:
            debug_start_cmd = gdb_start_cmd;
            debug_start_cmd [gdb_prog_index] = prog_name;
            debug_quit_str = gdb_quit_str;
                //
            quit_string = gdb_quit_str;
            break;
    }

    // create debugger pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) 
    {
        pfeme ("Debugger pipe failed");
    }

    // create debugger process
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        pfeme ("Debugger fork failed");
    }

    // parent process
    if (debugger_pid > 0) {

        // create debugger output process
        debug_out_pid = fork ();
        if (debug_out_pid == -1) {
            pfeme ("Debugger output fork failed");
        }
    }

    // debugger process
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe [PIPE_READ]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        dup2  (debug_out_pipe [PIPE_WRITE], STDERR_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);

        execvp (debug_start_cmd[0], debug_start_cmd);

        pfeme ("Debugger process failed");
    }

    // debugger output process
    if (debug_out_pid == 0) {

        close (debug_in_pipe [PIPE_READ]);
        close (debug_out_pipe [PIPE_WRITE]);

        // read pipe output
        while (1) 
        {
            // read output
            bytes_read = read (debug_out_pipe [PIPE_READ], 
                               debug_out_buffer, 
                               sizeof (debug_out_buffer) - 1);
            debug_out_buffer [bytes_read] = '\0';

            // check for debugger quit substring
            if (strstr (debug_out_buffer, debug_quit_str) != NULL) {
                break;
            }
        }

        // kill debugger process
        kill (debugger_pid, SIGTERM);

        exit (EXIT_SUCCESS);
    }
}



/*
    Set debugger
    ------
    Sets debug_state_t->debugger to corresponding macro
    based on file type  (data.h) 
*/
void set_debugger (debug_state_t *state, char *file_name)
{
    int file_type;
    unsigned char bytes[4];

    FILE *file = fopen (file_name, "rb");
    fread (bytes, 1, 4, file);

    // if ELF file -> gdb
        // 
    if (bytes[0] == 0x7f &&
        bytes[1] == 'E'  &&
        bytes[2] == 'L'  &&
        bytes[3] == 'F') 
    {
        state->debugger = DEBUGGER_GDB;
    } 

    // if unrecognized file type -> exit
    else {
        pfeme ("\"%s\": Unrecognized file type\n", file_name);
    }

    fclose (file);
}



void update_gdb_state (char* output, debug_state_t* state);
    //
    //
    //
void update_debugger_state (char* output,
                            debug_state_t* state)
{
    switch (state->debugger) {

        case DEBUGGER_GDB:
            update_gdb_state (output, state);
            break;
    }
}



void update_gdb_state (char* output,
                       debug_state_t* state)
{
    ptrdiff_t len;

        // output classes
    char *success   = "^done",
         *stopped   = "*stopped",
         *error     = "^error",

         // variables
         *path_var  = "fullname=\"",

         // misc
         *error_val = "none",
         *start, *end;

    // if ^done..., *stopped...
    if (strncmp (output, success, strlen (success)) == 0 ||
        strncmp (output, stopped, strlen (stopped)) == 0) 
    {
        // get current source file path
        //
            // find variable
        start = strstr(output, path_var);

            // get path
        if (start) {
            start += strlen(path_var);
            end = strchr(start, '\"');

            // add to state->src_path
            if (end) {
                len = end - start;
                if (len < sizeof (state->src_path)) {

                    // if changed
                    if (strcmp (start, state->src_path) != 0) {
                        strncpy (state->src_path, start, len);
                        state->src_path [len] = '\0';
                            //
                        state->src_file_changed = true;
                    }
                }
            }
        }
    } 
}
