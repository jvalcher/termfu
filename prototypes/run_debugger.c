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
#include "gdb.h"

#define DEBUGGER_UNKNOWN   0
#define DEBUGGER_GDB       1

#define PIPE_READ            0
#define PIPE_WRITE           1

#define PIPE_FAILED         -1
#define FORK_FAILED         -2
#define DEBUG_LAUNCH_FAILED -3
#define EXIT_PROGRAM        -5


int    debug_in_pipe  [2],
       debug_out_pipe [2];
pid_t  debugger_pid,
       debug_out_pid,
       ret_pid;


char *gdb_cmd[]      = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};
char *debugger_quit  = "^exit";


void start_debugger_reader (debug_state_t *dstate)
{

    debug_out_pid = fork ();
    if (debug_out_pid == -1) {
        perror ("Debugger output fork");
        exit (FORK_FAILED);
    }

    // debugger output process
    if (debug_out_pid == 0) {

        char    debug_out_buffer  [256],
                ch, nch;
        size_t  bytes_read;
        int     i;
        FILE   *out_file_ptr;
        bool    running,
                is_newline,
                is_output;

        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_WRITE]);

        //out_file_ptr = fopen("debug_out.log", "w");

        // read pipe output
        running = true;
        while (running) 
        {
            bytes_read = read (debug_out_pipe [PIPE_READ], 
                               debug_out_buffer, 
                               sizeof (debug_out_buffer) - 1);

            debug_out_buffer [bytes_read] = '\0';

            printf ("%s", debug_out_buffer);

            /*
            // parse output into file
            i = 0;
            is_output = false;
            is_newline = true;
            do {
                ch = debug_out_buffer[i++];
                if (is_newline && ch != '\n') {
                    is_newline = false;
                    if (ch == '~') {
                        is_output = true;
                    }
                    --bytes_read;
                    continue;
                }
                if (ch == '\n') {
                    is_newline = true;
                    is_output = false;
                    --bytes_read;
                    continue;
                }
                if (is_output) {
                    --bytes_read;
                    //fprintf (out_file_ptr, "%c", ch);
                    printf ("%c", ch);
                }
            } while (bytes_read > 0);
            */

            // exit
            if (strstr (debug_out_buffer, dstate->exit_str) != NULL) {
                running = false;
            }
        }

        //fclose (out_file_ptr);
    }
}


void start_debugger_process (debug_state_t *dstate)
{
    char  **cmd;

    // set command
    switch (dstate->debugger) {

        // GDB
        case (DEBUGGER_GDB):
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


void start_debugger (debug_state_t *dstate)
{
    // create debugger pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) 
    {
        perror("Debugger pipe");
        exit (PIPE_FAILED);
    }

    dstate->input_pipe  = debug_in_pipe [PIPE_WRITE];

    start_debugger_process (dstate);

    start_debugger_reader (dstate);
}


int main (int argc, char *argv[])
{
    int stdout_fd,
        dev_null_fd,
        ch;
    int bytes;
    char *prog,
         *break_loc;
    static struct termios oldt, newt;
    data_t *data;
    debug_state_t *dstate;

    // set program path
    if (argc < 2 || argc > 2) {
        printf ("Usage: a.out prog\n");
    } else {
        prog = argv[1];
    }

    // set breakpoint (TODO)
    break_loc = "main";

    // allocate structs
    data = (data_t*) malloc (sizeof (data_t)); 
    dstate = (debug_state_t*) malloc (sizeof (debug_state_t));
    data->debug_state = dstate;

    dstate->prog_path = prog;
    dstate->debugger = DEBUGGER_GDB;
    dstate->break_point = break_loc;
    dstate->out_done_str = "(gdb)";
    dstate->exit_str = "^exit";
    dstate->output_indicator = '~';
    //dstate->sem_lock = create_semaphore();

    // run debugger
    start_debugger(dstate);

    // run commands
    bool running = true;
    while (running) {

        tcgetattr (STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);          
        tcsetattr (STDIN_FILENO, TCSANOW, &newt);
        dup2 (dev_null_fd, STDOUT_FILENO);
            //
        ch = getchar();
            //
        dup2 (stdout_fd, STDOUT_FILENO);
        tcsetattr (STDIN_FILENO, TCSANOW, &oldt);

        // TODO: open semaphore

        switch (ch) {

            case 'r':
                gdb_run (dstate);
                break;
            case 'b':
                dstate->break_point = break_loc;
                gdb_set_breakpoint (dstate);
                break;
            case 'n':
                gdb_next (dstate);
                break;
            case 'c':
                gdb_continue (dstate);
                break;
            case 'l':
                gdb_get_local_vars (dstate);
                break;
            case 'q':
                gdb_exit (dstate);
                running = false;
        }
        // TODO: close semaphore
    }

    kill(debugger_pid, SIGTERM);
    kill(debug_out_pid, SIGTERM);
    waitpid(debugger_pid, NULL, 0);
    waitpid(debug_out_pid, NULL, 0);

    free (dstate);
    free (data);

    return 0;
}
