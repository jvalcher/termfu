#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>

#include "start_debugger.h"
#include "data.h"
#include "utilities.h"
#include "render_window_data.h"
#include "plugins/_interface.h"

#define PIPE_READ   0
#define PIPE_WRITE  1

int debug_in_pipe  [2],
    debug_out_pipe [2];

typedef struct win_update {
    char    code[4];
    WINDOW *win;
    char   *path;
    struct win_update *next;
} win_update_t;

static void create_debugger_pipes (debug_state_t *dstate);
static void set_debugger (debug_state_t *dstate);
static void start_debugger_proc (debug_state_t *dstate);
static void start_debugger_reader_proc (state_t *state);
static void update_window (plugin_t *plugins, char *code, char *debug_out_path);

// TODO: add to interface
char *gdb_cmd[]      = {"gdb", "--quiet", "--interpreter=mi", NULL, NULL};
char *debugger_quit  = "^exit";


/*
    Start debugger
*/
void start_debugger (state_t *state)
{
    debug_state_t *dstate = state->debug_state;

    create_debugger_pipes (dstate);

    set_debugger (dstate);

    start_debugger_proc (dstate);

    start_debugger_reader_proc (state);
}



static void create_debugger_pipes (debug_state_t *dstate)
{
    // create debugger pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1) 
    {
        perror("Debugger pipe");
        exit (EXIT_FAILURE);
    }
    dstate->input_pipe  = debug_in_pipe [PIPE_WRITE];
    dstate->output_pipe = debug_out_pipe [PIPE_READ];
}



// TODO: Get file type -> set debugger
static void set_debugger (debug_state_t *dstate)
{
    dstate->debugger = DEBUGGER_GDB;
}



/*
    Start debugger process
*/
static void start_debugger_proc (debug_state_t *dstate)
{
    char  **cmd;
    pid_t  debugger_pid;

    // set to running
    dstate->running = true;

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
        exit (EXIT_FAILURE);
    }

    if (debugger_pid == 0) {

        dup2  (debug_in_pipe  [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);
        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (cmd[0], cmd);

        perror ("Debugger process");
        exit (EXIT_FAILURE);
    }

}



/*
    Start debugger reading process
*/
static void start_debugger_reader_proc (state_t *state)
{
    pid_t          debug_out_pid;
    debug_state_t *dstate = state->debug_state;

    // fork
    debug_out_pid = fork ();
    if (debug_out_pid == -1) {
        perror ("Debugger output fork");
        exit (EXIT_FAILURE);
    }

    // debugger output process
    if (debug_out_pid == 0) {

        int     rstate;
        char    output_buffer [256],
                debug_out_path [256],
                program_out_path [256],
                code [4];
        size_t  bytes_read;
        bool    running;

        close (debug_in_pipe [PIPE_READ]);
        close (debug_in_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_WRITE]);

        // read pipe output
        running = true;
        while (running) 
        {
            bytes_read = read (debug_out_pipe [PIPE_READ], output_buffer, sizeof (output_buffer) - 1);
            output_buffer [bytes_read] = '\0';

            parse_output (&rstate, dstate->debugger, output_buffer, debug_out_path, program_out_path, code);

            // TODO: Handle program output

            switch (rstate) {
                case READER_RECEIVING:
                    break;
                case READER_DONE:
                    update_window (state->plugins, code, debug_out_path);
                    break;
                case READER_EXIT:
                    running = false;
                    break;
            }
        }

        exit (0);
    }

}



/*
    Update Ncurses plugin WINDOW data
*/
static void update_window (plugin_t *plugins, char *code, char *debug_out_path)
{
    int i, ch;
    plugin_t *plugin;
    window_t *win;
    WINDOW   *Win;
    FILE     *fp;

    // find code's plugin
    plugin = plugins;
    do {
        if (strcmp (plugin->code, code) == 0)
            break;
        plugin = plugin->next;
    } while (plugins != NULL);
    if (plugin == NULL){
        pfeme ("Unable to find plugin for code \"%s\"", code);
    }

    win = plugin->window;

    // open output file
    if (win->file_ptr) {
        fclose (win->file_ptr);
    }
    win->file_ptr  = fopen (plugin->data_file_path, "r");
    if (win->file_ptr == NULL) {
        pfeme ("Unable to open output file at \"%s\"", debug_out_path);
    }

    // get number of rows and max line length of file
    win->file_rows = 0;
    win->file_max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), win->file_ptr) != NULL) {
        line_len = strlen(line);
        if (win->file_max_cols < line_len) {
            win->file_max_cols = line_len + 1;
        }
        win->file_rows += 1;
    }
    rewind (win->file_ptr);

    // get file line byte offsets
    win->file_offsets = malloc ((size_t) win->file_rows * sizeof(long int));
    if (win->file_offsets == NULL) {
        pfeme ("Failed to allocate offsets array");
    }
    win->file_offsets [0] = 0;
    for (i = 1; i < win->file_rows; i++) {
        while ((ch = fgetc (win->file_ptr)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            win->file_offsets [i] = ftell (win->file_ptr);
    }

    // subtract borders
    win->win_rows -= 2;
    win->win_cols -= 2;

    // calculate min, max middle file lines for vertical scrolling
    win->file_min_mid = (win->win_rows / 2) + 1;
    win->file_max_mid = win->file_rows - ((win->win_rows - 1) / 2);

    // set initial window middle line
    win->win_mid_line = win->file_min_mid;

    // set first char to display for horizontal scrolling
    win->file_first_char = 0;

    // render window data starting from first line of file
    render_window_data (win, FIRST_OPEN);

    fclose (win->file_ptr);
}

