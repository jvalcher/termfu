#include <stddef.h>
#include <unistd.h>
#include <termio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>

#include "start_debugger.h"
#include "data.h"
#include "parse_debugger_output.h"
#include "utilities.h"
#include "plugins.h"
#include "insert_output_marker.h"
#include "update_window_data/_update_window_data.h"
#include "update_window_data/get_binary_path_time.h"

static int configure_debugger  (debugger_t*);
static int start_debugger_proc (state_t*);
static int send_setup_commands (state_t*);

bool debugger_configured = false;



int
start_debugger (state_t *state)
{
    int ret;

    if (debugger_configured == false) {
        ret = configure_debugger (state->debugger);
        if (ret == FAIL) {
            pfemr ("Failed to configure debugger");
        }
        debugger_configured = true;
    }

    ret = start_debugger_proc (state);
    if (ret == FAIL) {
        pfemr ("Failed to start debugger process");
    }

    ret = insert_output_end_marker (state);
    if (ret == FAIL) {
        pfemr (ERR_OUT_MARK);
    }

    ret = send_setup_commands (state);
    if (ret == FAIL) {
        pfemr ("Failed to send setup commands");
    }

    ret = parse_debugger_output (state);
    if (ret == FAIL) {
        pfemr (ERR_DBG_PARSE);
    }

    state->plugins[Dbg]->win->buff_data->new_data = true;
    ret = update_window (Dbg, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    ret = get_binary_path_time (state);
    if (ret == FAIL) {
        pfemr ("Failed to get binary path, time");
    }

    return A_OK;
}



static int
configure_debugger (debugger_t *debugger)
{
    // src_path_buffer
    if ((debugger->src_path_buffer = (char*) malloc (sizeof (char) * PROGRAM_PATH_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Source path buffer allocation failed");
    }
    debugger->src_path_buffer[0] = '\0';
    debugger->src_path_len = PROGRAM_PATH_LEN;
    debugger->src_path_pos = 0;
    debugger->src_path_times_doubled = 0;

    // main_src_path_buffer
    if ((debugger->main_src_path_buffer = (char*) malloc (sizeof (char) * PROGRAM_PATH_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Main source path buffer allocation failed");
    }
    debugger->main_src_path_buffer[0] = '\0';
    debugger->main_src_path_len = PROGRAM_PATH_LEN;
    debugger->main_src_path_pos = 0;
    debugger->main_src_path_times_doubled = 0;

    // format_buffer
    if ((debugger->format_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Format buffer allocation failed");
    }
    debugger->format_buffer[0] = '\0';
    debugger->format_len = ORIG_BUF_LEN;
    debugger->format_pos = 0;
    debugger->format_times_doubled = 0;

    // data_buffer
    if ((debugger->data_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Data buffer allocation failed");
    }
    debugger->data_buffer[0] = '\0';
    debugger->data_len = ORIG_BUF_LEN;
    debugger->data_pos = 0;
    debugger->data_times_doubled = 0;

    // cli_buffer
    if ((debugger->cli_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("CLI buffer allocation failed");
    }
    debugger->cli_buffer[0] = '\0';
    debugger->cli_len = ORIG_BUF_LEN;;
    debugger->cli_pos = 0;
    debugger->cli_times_doubled = 0;

    // program_buffer
    if ((debugger->program_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Program out buffer allocation failed");
    }
    debugger->program_buffer[0] = '\0';
    debugger->program_len = ORIG_BUF_LEN;;
    debugger->program_pos = 0;
    debugger->program_times_doubled = 0;

    // async_buffer
    if ((debugger->async_buffer = (char*) malloc (sizeof (char) * ORIG_BUF_LEN)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Async buffer allocation failed");
    }
    debugger->async_buffer[0] = '\0';
    debugger->async_len = ORIG_BUF_LEN;
    debugger->async_pos = 0;
    debugger->async_times_doubled = 0;

    return A_OK;
}



static int
start_debugger_proc (state_t *state)
{
    pid_t   debugger_pid;
    int     i,
            debug_in_pipe  [2],
            debug_out_pipe [2];

    debugger_t *debugger = state->debugger;
    
    // create pipes
    if (pipe (debug_in_pipe)  == -1 || 
        pipe (debug_out_pipe) == -1)
    {
        pfemr ("Debugger pipe creation failed");
    }
    debugger->stdin_pipe  = debug_in_pipe [PIPE_WRITE];
    debugger->stdout_pipe = debug_out_pipe [PIPE_READ];

    // fork
    debugger_pid = fork ();
    if (debugger_pid  == -1)
    {
        pfemr ("Debugger fork failed");
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

        i = 0;
        do {
            strcat (state->debugger->format_buffer, state->command[i++]);
            strcat (state->debugger->format_buffer, " ");
        } while (state->command [i] != NULL);
        pfemr ("execvp error with command: \"%s\"", state->debugger->format_buffer);
    }

    if (debugger_pid > 0) {
    
        debugger->pid = debugger_pid;
        debugger->running = true;

        close (debug_in_pipe   [PIPE_READ]);
        close (debug_out_pipe  [PIPE_WRITE]);
    }

    return A_OK;
}



static int
send_setup_commands (state_t *state)
{
    int   ret;
    char *cmd_confirm_off = "set confirm off\n";

    switch (state->debugger->index) {
        case DEBUGGER_GDB:
            ret = send_command_mp (state, cmd_confirm_off);
            if (ret == FAIL) {
                pfemr ("Failed to send GDB setup command");
            }
            break;
    }

    return A_OK;
}
