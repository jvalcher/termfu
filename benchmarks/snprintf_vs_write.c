
/*
    Benchmark:  snprintf() write()  vs.  write() write() write()
                    (winner)
*/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

#define READ   0
#define WRITE  1

#define NUM_STR_WRITES 1000000


static void measure_time (const char *message, struct timespec start, struct timespec end) {
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("%s: \t%f seconds\n", message, time_taken);
}


int main (void)
{
    int in_pipe [2],
        sync_pipe [2];

    pipe (in_pipe);
    pipe (sync_pipe);

    pid_t pid = fork();
    if (pid == -1) {
        fprintf (stderr, "Fork failed");
    }

    else if (pid == 0) {

        bool reading = true;
        char buffer[128];

        close (in_pipe [WRITE]);
        close (sync_pipe [READ]);
        write (sync_pipe [WRITE], "READY\n", strlen ("READY\n"));

        while (reading) {
            ssize_t bytes = read (in_pipe [READ], buffer, sizeof (buffer) - 1);
            buffer [bytes] = '\0';
            if (strstr (buffer, "EXIT") != NULL) {
                reading = false;
            }
        }

    } else {

        char str [128];
        struct timespec start, end;

        char *str1 = "I love sarcasm. ";
        char *str2 = "It\'s like punching people in the face, ";
        char *str3 = "but with words.\n";
        char *exit_str = "EXIT";

        close (in_pipe [READ]);
        close (sync_pipe [WRITE]);

        read (sync_pipe [READ], str, sizeof (str));

        // snprintf() + write()
        puts ("");
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < NUM_STR_WRITES; i++) {
            snprintf (str, sizeof (str), "%s%s%s", str1, str2, str3);
            write  (in_pipe [WRITE], str, strlen (str));
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        measure_time("Time with snprintf", start, end);

        // write() write() write()
        clock_gettime(CLOCK_MONOTONIC, &start);
        for (int i = 0; i < NUM_STR_WRITES; i++) {
            write  (in_pipe [WRITE], str1, strlen (str));
            write  (in_pipe [WRITE], str2, strlen (str));
            write  (in_pipe [WRITE], str3, strlen (str));
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        measure_time("Time with writes", start, end);
        puts ("");

        write  (in_pipe [WRITE], exit_str, strlen (exit_str));
        wait (NULL);

        close (in_pipe [WRITE]);
    }

    return 0;
}
