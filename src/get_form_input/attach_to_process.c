#include "../data.h"
#include "../plugins.h"
#include "../update_window_data/_update_window_data.h"
#include "_get_form_input.h"
#include "../utilities.h"
#include "../error.h"

#define PID_BUF_LEN  8



int
attach_to_process (state_t *state)
{
    char *cmd;

    if (get_form_input ("Process ID: ", state->input_buffer) == FAIL)
        pfemr (ERR_POPUP_IN);

    // attach to process
    cmd = concatenate_strings (3, "attach ", state->input_buffer, "\n");
    if (send_command_mp (state, cmd) == FAIL)
        pfemr ("Failed to attach to debugged process ID \"%s\"", state->input_buffer);
    free (cmd);

    state->plugins[Dbg]->win->buff_data->new_data = true;
    if (update_window (Dbg) == FAIL)
        pfemr (ERR_UPDATE_WINS);

    return A_OK;
}

