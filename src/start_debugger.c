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
#include "error.h"
#include "plugins.h"
#include "update_window_data/_update_window_data.h"
#include "update_window_data/get_binary_path_time.h"

static int  configure_debugger       (debugger_t*);
static void start_debugger_proc      (state_t*);
static int  send_setup_commands      (state_t*);

bool vm_started = false;

bool debugger_configured = false;



int
start_debugger (state_t *state)
{
    if (debugger_configured == false)
        if (configure_debugger (state->debugger) == FAIL)
            pfemr ("Failed to configure debugger");

    start_debugger_proc (state);

    if (insert_output_end_marker (state) == FAIL)
        pfemr (ERR_OUT_MARK);

    if (send_setup_commands (state) == FAIL)
        pfemr ("Failed to send setup commands");

    if (parse_debugger_output (state) == FAIL)
        pfemr (ERR_DBG_PARSE);

    state->plugins[Dbg]->win->buff_data->new_data = true;

    if (update_window (Dbg) == FAIL)
        pfemr (ERR_UPDATE_WIN);

    if (get_binary_path_time (state) == FAIL)
        pfemr ("Failed to get binary path, time");

    return A_OK;
}



static int
configure_debugger (debugger_t *debugger)
{
    // src_path_buffer
    if ((debugger->src_path_buffer = (char*) malloc (sizeof (char) * PROGRAM_PATH_LEN)) == NULL)
        pfemr_errno ("Source path buffer allocation failed");
    debugger->src_path_buffer[0] = '\0';
    debugger->src_path_len = PROGRAM_PATH_LEN;
    debugger->src_path_pos = 0;
    debugger->src_path_times_doubled = 0;

    // main_src_path_buffer
    if ((debugger->main_src_path_buffer = (char*) malloc (sizeof (char) * PROGRAM_PATH_LEN)) == NULL)
        pfemr_errno ("Main source path buffer allocation failed");
    debugger->main_src_path_buffer[0] = '\0';
    debugger->main_src_path_len = PROGRAM_PATH_LEN;
    debugger->main_src_path_pos = 0;
    debugger->main_src_path_times_doubled = 0;

    // format_buffer
    if ((debugger->format_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL)
        pfemr_errno ("Format buffer allocation failed");
    debugger->format_buffer[0] = '\0';
    debugger->format_len = ORIG_BUF_LEN;
    debugger->format_pos = 0;
    debugger->format_times_doubled = 0;

    // data_buffer
    if ((debugger->data_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL)
        pfemr_errno ("Data buffer allocation failed");
    debugger->data_buffer[0] = '\0';
    debugger->data_len = ORIG_BUF_LEN;
    debugger->data_pos = 0;
    debugger->data_times_doubled = 0;

    // cli_buffer
    if ((debugger->cli_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL)
        pfemr_errno ("CLI buffer allocation failed");
    debugger->cli_buffer[0] = '\0';
    debugger->cli_len = ORIG_BUF_LEN;;
    debugger->cli_pos = 0;
    debugger->cli_times_doubled = 0;

    // program_buffer
    if ((debugger->program_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL)
        pfemr_errno ("Program out buffer allocation failed");
    debugger->program_buffer[0] = '\0';
    debugger->program_len = ORIG_BUF_LEN;;
    debugger->program_pos = 0;
    debugger->program_times_doubled = 0;

    // async_buffer
    if ((debugger->async_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL)
        pfemr_errno ("Async buffer allocation failed");
    debugger->async_buffer[0] = '\0';
    debugger->async_len = ORIG_BUF_LEN;
    debugger->async_pos = 0;
    debugger->async_times_doubled = 0;

    return A_OK;
}



static void
start_debugger_proc (state_t *state)
{
    pid_t  debugger_pid;
    int    debug_in_pipe  [2],
           debug_out_pipe [2];

    debugger_t *debugger = state->debugger;
    
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || pipe (debug_out_pipe) == -1)
        pfeme ("Debugger pipe creation failed");
    debugger->stdin_pipe  = debug_in_pipe [PIPE_WRITE];
    debugger->stdout_pipe = debug_out_pipe [PIPE_READ];

    // fork
    debugger_pid = fork ();
    if (debugger_pid  == -1)
        pfeme ("Debugger fork failed");

    // start debugger
    if (debugger_pid == 0) {

        dup2  (debug_in_pipe  [PIPE_READ], STDIN_FILENO);
        close (debug_in_pipe  [PIPE_READ]);
        close (debug_in_pipe  [PIPE_WRITE]);

        dup2  (debug_out_pipe [PIPE_WRITE], STDOUT_FILENO);
        close (debug_out_pipe [PIPE_WRITE]);
        close (debug_out_pipe [PIPE_READ]);

        execvp (state->command[0], state->command);

        pfeme_errno ("Failed to start debugger \"%s\"", debugger->title);
    }

    if (debugger_pid > 0) {
    
        debugger->pid = debugger_pid;
        debugger->running = true;

        close (debug_in_pipe   [PIPE_READ]);
        close (debug_out_pipe  [PIPE_WRITE]);
    }
}



static int
send_setup_commands (state_t *state)
{
    char *cmd_confirm_off = "set confirm off\n";
    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            if (send_command_mp (state, cmd_confirm_off) == FAIL)
                pfemr ("Failed to send GDB setup command");
            break;
    }
    return A_OK;
}

