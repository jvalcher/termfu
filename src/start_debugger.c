#include <stddef.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "start_debugger.h"
#include "data.h"
#include "parse_debugger_output.h"
#include "utilities.h"
#include "plugins.h"
#include "insert_output_marker.h"
#include "update_window_data/_update_window_data.h"
#include "update_window_data/get_binary_path_time.h"

static void configure_debugger (debugger_t*);
static void start_debugger_proc (state_t*);



void
start_debugger (state_t *state)
{
    configure_debugger (state->debugger);

    start_debugger_proc (state);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    state->plugins[Dbg]->win->buff_data->new_data = true;
    update_window (Dbg, state);

    get_binary_path_time (state);
}



static void
configure_debugger (debugger_t *debugger)
{
    debugger->format_buffer[0] = '\0';
    debugger->format_len = FORMAT_BUF_LEN;
    debugger->format_pos = 0;

    debugger->data_buffer[0] = '\0';
    debugger->data_len = DATA_BUF_LEN;
    debugger->data_pos = 0;

    debugger->cli_buffer[0] = '\0';
    debugger->cli_len = CLI_BUF_LEN;
    debugger->cli_pos = 0;

    debugger->program_buffer[0] = '\0';
    debugger->program_len = PROGRAM_BUF_LEN;
    debugger->program_pos = 0;

    debugger->async_buffer[0] = '\0';
    debugger->async_len = ASYNC_BUF_LEN;
    debugger->async_pos = 0;
}



static void
start_debugger_proc (state_t *state)
{
    pid_t   debugger_pid;
    int     debug_in_pipe  [2],
            debug_out_pipe [2];

    debugger_t *debugger = state->debugger;
    
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1)
    {
        pfeme ("Debugger pipe creation failed\n");
    }
    debugger->stdin_pipe  = debug_in_pipe [PIPE_WRITE];
    debugger->stdout_pipe = debug_out_pipe [PIPE_READ];

    // fork
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        pfeme ("Debugger fork failed\n");
    }

    // start debugger
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe  [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);

        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (state->command[0], state->command);

        pfeme ("Debugger start failed\n");
    }

    if (debugger_pid > 0) {
    
        debugger->running = true;

        close (debug_in_pipe   [PIPE_READ]);
        close (debug_out_pipe  [PIPE_WRITE]);
    }
}

