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

#define PIPE_READ            0
#define PIPE_WRITE           1
#define PIPE_FAILED         -1
#define FORK_FAILED         -2
#define DEBUG_LAUNCH_FAILED -3
#define OUTPUT_FILE_FAILED  -4
#define EXIT_PROGRAM        -1

char *debugger[]           = {"gdb", "--quiet", NULL};
char *debugger_quit        = "quit";
char *debug_prog           = "hello";
char *breakpoint           = "main";
char *debug_out_file       = "debug_out.log";
char *debug_prog_out_file  = "debug_prog_out.log";
char *gdb_prompt           = "(gdb) ";

int    debug_in_pipe  [2],
       debug_out_pipe [2];
pid_t  debugger_pid,
       debug_out_pid;


void remove_substring (char *source, const char *substring) {
    size_t substring_len = strlen(substring);
    char *found_position;
    while ((found_position = strstr(source, substring)) != NULL) {
        memmove(found_position, 
                found_position + substring_len, 
                strlen(found_position + substring_len) + 1);
    }
}


void run_debugger (void)
{
    char   debug_out_buffer [256];
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

    if (debugger_pid > 0) {
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

            remove_substring (debug_out_buffer, gdb_prompt); 

            printf ("%s", debug_out_buffer);

            // check if debugger quit command
            if (strstr (debug_out_buffer, debugger_quit) != NULL) {
                break;
            }
        }
    }
}


ssize_t run_debugger_cmd (char ch)
{
    int      size;
    ssize_t  bytes;

    // gdb commands
    char *gdb_confirm_off        = "set confirm off\n",
         *gdb_continue           = "continue\n",
         *gdb_next               = "next\n",
         *gdb_quit               = "quit\n",
         *gdb_run,
         *gdb_break,
         *gdb_file;

    // gdb_file
    size = snprintf (NULL, 0, "%s %s\n", "file", debug_prog) + 1;
    gdb_file = malloc(size);
    if (gdb_file == NULL) {
        perror("gdb run malloc");
        return -1;
    }
    snprintf (gdb_file, size, "%s %s\n", "file", debug_prog);

    // gdb_run
    size = snprintf (NULL, 0, "%s > %s\n", "run", debug_prog_out_file) + 1;
    gdb_run = malloc(size);
    if (gdb_run == NULL) {
        perror("gdb run malloc");
        return -1;
    }
    snprintf (gdb_run, size, "%s > %s\n", "run", debug_prog_out_file);

    // gdb_break
    size = snprintf (NULL, 0, "%s %s\n", "break", breakpoint) + 1;
    gdb_break = malloc(size);
    if (gdb_break == NULL) {
        perror("gdb break malloc");
        return -1;
    }
    snprintf (gdb_break, size, "%s %s\n", "break", breakpoint);

    switch (ch) {
        case 'f':
            printf ("(gdb) %s", gdb_file);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_file, strlen(gdb_file));
            return bytes;
        case 'r':
            printf ("(gdb) %s", gdb_run);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_run, strlen(gdb_run));
            return bytes;
        case 'b':
            printf ("(gdb) %s", gdb_break);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_break, strlen(gdb_break));
            return bytes;
        case 'n':
            printf ("(gdb) %s", gdb_next);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_next, strlen(gdb_next));
            return bytes;
        case 'c':
            printf ("(gdb) %s", gdb_continue);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_continue, strlen(gdb_continue));
            return bytes;
        case 'q':
            printf ("(gdb) %s", gdb_confirm_off);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_confirm_off, strlen(gdb_confirm_off));
            usleep (250000);
            printf ("(gdb) %s", gdb_quit);
            bytes = write (debug_in_pipe [PIPE_WRITE], gdb_quit, strlen(gdb_quit));
            return EXIT_PROGRAM;
    }

    return EXIT_FAILURE;
}


int main (void)
{
    int stdout_fd,
        dev_null_fd,
        ch;
    size_t bytes;
    static struct termios oldt, newt;

    stdout_fd = dup (STDOUT_FILENO);
    dev_null_fd = open("/dev/null", O_WRONLY);

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

        if ((int) bytes == EXIT_PROGRAM) {
            break;
        } else if (bytes <= 0) {
            fprintf (stderr, "Debugger command failed (%lu)\n", bytes);
        }
    }
}
