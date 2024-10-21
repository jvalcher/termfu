#include "../data.h"
#include "../plugins.h"
#include "../update_window_data/_update_window_data.h"
#include "../utilities.h"

#define PID_BUF_LEN  8



int
attach_to_process (state_t *state)
{
    FILE *fp;
    int   ch, i, ret;
    char  pid_buff [PID_BUF_LEN],
         *cmd;

    // get PID
    if ((fp = fopen (DEBUG_PID_FILE, "r")) == NULL) {
        pfem ("fopen error: \"%s\"", strerror (errno));
        pemr ("Failed to open debugged process PID file \"%s\"", DEBUG_PID_FILE);
    }
    i = 0;
    while ((ch = fgetc (fp)) != EOF && i < (PID_BUF_LEN - 2)) {
        pid_buff [i++] = ch;
    }
    pid_buff [i] = '\0';
    fclose (fp);

    // attach to process
    cmd = concatenate_strings (3, "attach ", pid_buff, "\n");
    ret = send_command_mp (state, cmd);
    if (ret == FAIL) {
        pfemr ("Failed to attach to debugged process");
    }
    free (cmd);

    state->plugins[Dbg]->win->buff_data->new_data = true;
    ret = update_window (Dbg, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WINS);
    }

    return A_OK;
}

