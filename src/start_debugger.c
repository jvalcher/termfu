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
#include "utilities.h"
#include "parse_debugger_output.h"
#include "update_window_data.h"


static void  create_debugger_pipes       (debug_state_t*);
static void  create_data_dir             (void);
static void  set_debugger                (debug_state_t*);
static void  start_debugger_proc         (debug_state_t*);
static void  start_debugger_reader_proc  (state_t*);

#define PIPE_READ   0
#define PIPE_WRITE  1
int debug_in_pipe  [2],
    debug_out_pipe [2];
bool first_run;

// TODO: 
//  - get this via plugins/_interface
//  - add more gdb flags
# define GDB_PROG_INDEX  3
char *gdb_cmd[] = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};



void start_debugger (state_t *state)
{
    set_debugger (state->debug_state);

    create_debugger_pipes (state->debug_state);

    create_data_dir ();

    start_debugger_proc (state->debug_state);

    start_debugger_reader_proc (state);
}



static void set_debugger (debug_state_t *dstate)
{
    // TODO: Get file type -> set debugger
    dstate->debugger = DEBUGGER_GDB;
}



static void create_debugger_pipes (debug_state_t *dstate)
{
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) 
    {
        perror("Debugger pipe");
        exit (EXIT_FAILURE);
    }

    dstate->input_pipe  = debug_in_pipe [PIPE_WRITE];
    dstate->output_pipe = debug_out_pipe [PIPE_READ];
}



static void create_data_dir (void)
{
    char  path [256];
    struct stat st = {0};
    char *home = getenv ("HOME");

    // create directory
    snprintf (path, sizeof (path), "%s/%s", home, DATA_DIR_PATH);
    if (stat (path, &st) == -1)
        mkdir (path, 0700);
}



/*
    Start debugger process
*/
static void start_debugger_proc (debug_state_t *dstate)
{
    char  **cmd;
    pid_t  debugger_pid;

    // set command
    switch (dstate->debugger) {
        case (DEBUGGER_GDB):
            cmd = gdb_cmd;      // TODO: see globals
            cmd [GDB_PROG_INDEX] = dstate->prog_path;
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

}



/*
    Start debugger reading process
*/
static void start_debugger_reader_proc (state_t *state)
{
    pid_t debug_out_pid;

    // fork
    debug_out_pid = fork ();
    if (debug_out_pid == -1) {
        perror ("Debugger output fork");
        exit (EXIT_FAILURE);
    }

    // start debugger reader
    if (debug_out_pid == 0) {

        int            reader_state;
        char           output_buffer [1024],
                       plugin_code [4],
                       debug_out_path [256],
                       program_out_path [256],
                      *path;
        size_t         bytes_read;
        bool           running;
        debug_state_t *dstate;


        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_WRITE]);

        reader_state = READER_RECEIVING;

        dstate = state->debug_state;

        // set initial output file paths
        path = get_code_path ("Prm", state->plugins);
        strncpy (debug_out_path, path, strlen (path) + 1);
        path = get_code_path ("Out", state->plugins);
        strncpy (program_out_path, path, strlen (path) + 1);

        // reader loop
        running = true;
        while (running) 
        {
            // read debugger pipe output
            bytes_read = read (debug_out_pipe [PIPE_READ], 
                               output_buffer, 
                               sizeof (output_buffer) - 1);
            output_buffer [bytes_read] = '\0';

#ifdef DEBUG
            printf ("%s", output_buffer);
#endif

            parse_debugger_output ( dstate->debugger, 
                                   &reader_state, 
                                    output_buffer, 
                                    plugin_code, 
                                    debug_out_path, 
                                    program_out_path);

            switch (reader_state) {

                case READER_RECEIVING:
                    break;

                case READER_DONE:
                    update_window_data (state, plugin_code);
                    update_window_data (state, "Out");

                    break;

                case READER_EXIT:
                    running = false;
                    break;
            }
        }
        exit (EXIT_SUCCESS);
    }

}



