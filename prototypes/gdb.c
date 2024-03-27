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

#define PIPE_READ            0
#define PIPE_WRITE           1
#define PIPE_FAILED         -1
#define FORK_FAILED         -2
#define DEBUG_LAUNCH_FAILED -3
#define OUTPUT_FILE_FAILED  -4
#define EXIT_PROGRAM        -5
#define INVALID_CH          -6

char *debugger[]           = {"gdb", "--quiet", "--interpreter=mi", NULL};
char *debugger_quit        = "^exit";
char *debug_prog           = "hello";
char *breakpoint           = "main";
char *debug_out_file       = "debug_out.log";
char *debug_prog_out_file  = "debug_prog_out.log";
char *gdb_prompt           = "(gdb) ";

// gdb commands
char *gdb_confirm_off = "-gdb-set confirm off\n",
     *gdb_continue    = "-exec-continue\n",
     *gdb_next        = "-exec-next\n",
     *gdb_step        = "-exec-step\n",
     *gdb_quit        = "-gdb-exit\n",
     *gdb_run         = "-exec-run\n",
     *gdb_break       = "-break-insert",
     *gdb_quit_string = "^exit",
     *gdb_break,
     *gdb_file;

int    debug_in_pipe  [2],
       debug_out_pipe [2];
pid_t  debugger_pid,
       debug_out_pid,
       ret_pid;



void run_debugger (void)
{
    char   debug_out_buffer [256];
    int status,
        exit_status;
    size_t bytes_read;

    // create debugger pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) 
    {
        perror("Debugger pipe");
        exit (PIPE_FAILED);
    }

    printf ("Starting debugger...\n");

    // create debugger process
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        perror ("Debugger fork");
        exit (FORK_FAILED);
    }

    // create debugger output process
    if (debugger_pid > 0) {
        debug_out_pid = fork ();
        if (debug_out_pid == -1) {
            perror ("Debugger output fork");
            exit (FORK_FAILED);
        }
    }

        // debugger process
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe [PIPE_READ]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        dup2  (debug_out_pipe [PIPE_WRITE], STDERR_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);

        execvp (debugger[0], debugger);

        perror ("Debugger process");
        exit (DEBUG_LAUNCH_FAILED);
    }

        // debugger output process
    if (debug_out_pid == 0) {

        close (debug_in_pipe [PIPE_READ]);
        close (debug_out_pipe [PIPE_WRITE]);

        // read pipe output
        while (1) 
        {
            bytes_read = read (debug_out_pipe [PIPE_READ], 
                               debug_out_buffer, 
                               sizeof (debug_out_buffer) - 1);
            debug_out_buffer [bytes_read] = '\0';

            printf ("%s", debug_out_buffer);

            // check if debugger quit command
            if (strstr (debug_out_buffer, debugger_quit) != NULL) {
                break;
            }
        }
        kill (debugger_pid, SIGTERM);
        exit (EXIT_SUCCESS);
    }
}


int run_debugger_cmd (char ch)
{
    ssize_t  bytes;

    switch (ch) {
        case 'f':
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_file, strlen(gdb_file));
            return (int)bytes;
        case 'r':
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_run, strlen(gdb_run));
            return (int)bytes;
        case 'b':
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_break, strlen(gdb_break));
            return (int)bytes;
        case 'n':
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_next, strlen(gdb_next));
            return (int)bytes;
        case 'c':
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_continue, strlen(gdb_continue));
            return (int)bytes;
        case 'q':
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_quit, strlen(gdb_quit));
            return EXIT_PROGRAM;
    }

    return INVALID_CH;
}


int main (void)
{
    int stdout_fd,
        dev_null_fd,
        ch,
        size;
    int bytes;
    static struct termios oldt, newt;

    stdout_fd = dup (STDOUT_FILENO);
    dev_null_fd = open("/dev/null", O_WRONLY);

    // gdb_file
    size = snprintf (NULL, 0, "%s %s\n", "-file-exec-and-symbols", debug_prog) + 1;
    gdb_file = malloc(size);
    if (gdb_file == NULL) {
        perror("gdb run malloc");
        return -1;
    }
    snprintf (gdb_file, size, "%s %s\n", "-file-exec-and-symbols", debug_prog);

    // gdb_break
    size = snprintf (NULL, 0, "%s %s\n", "-break-insert", breakpoint) + 1;
    gdb_break = malloc(size);
    if (gdb_break == NULL) {
        perror("gdb break malloc");
        return -1;
    }
    snprintf (gdb_break, size, "%s %s\n", "-break-insert", breakpoint);

    // run debugger
    run_debugger();

    while (1) {

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

        bytes = run_debugger_cmd(ch);

        if (ch == 'q' || bytes == EXIT_PROGRAM) {
            break;
        }
    }

    free(gdb_break);
    free(gdb_file);

    return 0;
}
