#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/mman.h>

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

#define BUFF_SIZE  1024
#define SHM_KEY    0x

int    debug_in_pipe  [2],
       debug_out_pipe [2],
       win_code_pipe  [2];
pid_t  debugger_pid,
       debug_out_pid,
       ret_pid;

char *gdb_cmd[]      = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};
char *debugger_quit  = "^exit";

char *code_file_path = "./curr_code.log";

char *debug_out_path = "./debug.out";
char *program_out_path = "./program.out";
char *win_update_code = "./win_update.code";

void start_debugger_process (debug_state_t *dstate);
void start_debugger_reader (debug_state_t *dstate);
void start_debugger (debug_state_t *dstate);
void update_reader_code (char *path, char *new_code);
void get_reader_code (char *path, char *code);



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

    // run debugger
    start_debugger(dstate);

    // run commands
    int i = 0;
    char *code[] = {"AbC", "DcS", "eFE", "asD"};
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

        update_reader_code (code_file_path, code [i++]);
        if (i > 3) i = 0;

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
                break;
        }
    }

    puts ("");
    puts ("----------");
    puts ("");

    // print files to stdout
    printf ("PROGRAM OUT\n");
    FILE *program_out_ptr = fopen (program_out_path, "r");
    while ((ch = fgetc(program_out_ptr)) != EOF) {
        printf ("%c", ch);
    }
    fclose (program_out_ptr);
    puts("");

    printf ("DEBUG OUT\n");
    FILE *debug_out_ptr = fopen (debug_out_path, "r");
    while ((ch = fgetc(debug_out_ptr)) != EOF) {
        printf ("%c", ch);
    }
    fclose (debug_out_ptr);
    puts("");

    free (dstate);
    free (data);

    return 0;
}



void start_debugger (debug_state_t *dstate)
{
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) {

        perror("Debugger pipe");
        exit (PIPE_FAILED);
    }

    dstate->input_pipe    = debug_in_pipe [PIPE_WRITE];

    start_debugger_process (dstate);

    start_debugger_reader (dstate);
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



void start_debugger_reader (debug_state_t *dstate)
{

    debug_out_pid = fork ();
    if (debug_out_pid == -1) {
        perror ("Debugger output fork");
        exit (FORK_FAILED);
    }

    // debugger output process
    if (debug_out_pid == 0) {

        char    output_buffer [4096],
                //debug_out_buffer [256],
                //program_out_buffer [256],
                code [4];
        size_t  bytes_read;
        int     i, j;
        FILE   *out_file_ptr;
        bool    running;
        FILE   *program_out_ptr,
               *debug_out_ptr;

        // close unneeded debugger pipes
        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_WRITE]);

        // clear output files
        debug_out_ptr = fopen (debug_out_path, "w");
        program_out_ptr = fopen (program_out_path, "w");
        fclose (debug_out_ptr);
        fclose (program_out_ptr);

        // read pipe output
        running = true;
        while (running) 
        {
            // get debugger output
            bytes_read = read (debug_out_pipe [PIPE_READ], 
                               output_buffer, 
                               sizeof (output_buffer) - 1);

            output_buffer [bytes_read] = '\0';
            //debug_out_buffer [0] = '\0';
            //program_out_buffer [0] = '\0';

            //printf ("%d\n", bytes_read);
            printf ("%s", output_buffer);

            get_reader_code (code_file_path, code);

            gdb_parse_output (output_buffer, debug_out_path, program_out_path);
            
            //printf ("%s", debug_out_buffer);
            //printf ("%s", program_out_buffer);

            // output done
            if (strstr (output_buffer, dstate->out_done_str) != NULL) {

            }

            // exit
            if (strstr (output_buffer, dstate->exit_str) != NULL) {
                running = false;
            }
        }

        //fclose (out_file_ptr);
        exit (0);
    }

}



void update_reader_code (char *path, char *new_code)
{
    FILE *file = fopen (path, "w");
    fwrite (new_code, 1, strlen (new_code) + 1, file);
    fclose (file);
}



void get_reader_code (char *path, char *code)
{
    FILE *file = fopen (path, "r");
    fread (code, 1, strlen (code) + 1, file);
    fclose (file);
}
