#include <stdlib.h>

#include "../data.h"
#include "../utilities.h"
#include "../error.h"
#include "../plugins.h"

static int get_watchpoint_data_gdb (state_t *state);
static int get_watchpoint_data_pdb (state_t *state);

#define INDEX_BUF_LEN  8



int
get_watchpoint_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            if (get_watchpoint_data_gdb (state) == FAIL)
                pfemr ("Failed to get watchpoint data (GDB)");
            break;
        case (DEBUGGER_PDB):
            if (get_watchpoint_data_pdb (state) == FAIL)
                pfemr ("Failed to get watchpoint data (GDB)");
            break;
    }
    return A_OK;
}



static int
get_watchpoint_data_gdb (state_t *state)
{
    int           i,
                  last_char_offset;
    char         *cmd, *ptr,
                 *src_ptr,
                 *data_ptr,
                 *watch_val,
                 *dest_ptr,
                  index_buff [INDEX_BUF_LEN];
    buff_data_t  *dest_data;
    window_t     *win;
    watchpoint_t *watch;

    char *hex = "0x";

    win       = state->plugins[Wat]->win;
    dest_data = win->buff_data;
    watch     = state->watchpoints;

    win->buff_data->buff_pos = 0;
    win->buff_data->buff[0] = '\0';
    win->buff_data->changed = true;

    // no watchpoints
    if (watch == NULL) {
        cp_wchar (dest_data, '\0');
    }

    // watchpoints
    else {
        while (watch != NULL) {

            // send watchpoint command
            cmd = concatenate_strings (3, "print ", watch->var, "\n");    
            if (send_command_mp (state, cmd) == FAIL)
                pfemr (ERR_DBG_CMD);
            free (cmd);

            src_ptr  = state->debugger->cli_buffer;
            data_ptr = state->debugger->data_buffer;
            watch_val = watch->value;

            cp_wchar (dest_data, '(');

            // index
            i = 0;
            if (snprintf (index_buff, INDEX_BUF_LEN - 1, "%d", watch->index) < 0)
                pfemr ("Failed to convert index int to string");
            while (index_buff [i] != '\0')
                cp_wchar (dest_data, index_buff [i++]);

            cp_wchar (dest_data, ')');
            cp_wchar (dest_data, ' ');

            // variable
            i = 0;
            while (watch->var[i] != '\0')
                cp_wchar (dest_data, watch->var[i++]);

            cp_wchar (dest_data, ' ');
            cp_wchar (dest_data, '=');
            cp_wchar (dest_data, ' ');

            if (strstr (data_ptr, "error") == NULL) {

                // skip hex value or '='
                ptr = src_ptr;
                src_ptr = strstr (src_ptr, hex);
                if (src_ptr != NULL) {
                    i = 0;
                    while (*src_ptr != ' ') {
                        cp_wchar (dest_data, *src_ptr);
                        if (i < WATCH_LEN) {
                            watch_val [i++] = *src_ptr;
                        }
                        ++src_ptr;
                    }
                    watch_val [i] = '\0';
                } else {
                    src_ptr = ptr;
                    src_ptr = strstr (src_ptr, "=");
                    src_ptr += 1;
                }

                src_ptr = strstr (src_ptr, " ");

                // copy value
                i = 0;
                while (*src_ptr != '\n') {
                    cp_wchar (dest_data, *src_ptr);
                    if (i < WATCH_LEN) {
                        watch_val [i++] = *src_ptr;
                    } 
                    ++src_ptr;
                }
                watch_val [i] = '\0';

                // remove trailing newline character
                dest_ptr = win->buff_data->buff;
                last_char_offset = win->buff_data->buff_pos - 1;
                if (*(dest_ptr + last_char_offset) == 'n' && *(dest_ptr + (last_char_offset - 1)) == '\\') {
                    win->buff_data->buff_pos -= 2;
                }

                cp_wchar (dest_data, '\n');
            }

            else {
                strcpy (watch->value, "none");
                i = 0;
                while (watch->value [i] != '\0') {
                    cp_wchar (dest_data, watch->value [i++]);
                }
                cp_wchar (dest_data, '\n');
            }

            state->debugger->cli_buffer[0] = '\0';
            state->debugger->data_buffer[0] = '\0';
            watch = watch->next;
        }
    }

    return A_OK;
}



static int
get_watchpoint_data_pdb (state_t *state)
{
    char         *cmd,
                 *var_ptr,
                 *cli_ptr,
                 *prog_ptr,
                  index_buff [24];
    window_t     *win;
    watchpoint_t *watch;
    buff_data_t  *dest_data;

    char *name_err = "*** NameError",
         *none_str = "none\n";

    win       = state->plugins[Wat]->win;
    dest_data = win->buff_data;
    watch     = state->watchpoints;

    dest_data->buff_pos = 0;
    dest_data->buff[0] = '\0';

    if (watch != NULL) {

        while (watch != NULL) {

            cmd = concatenate_strings (3, "p ", watch->var, "\n");    
            if (send_command_mp (state, cmd) == FAIL)
                pfemr (ERR_DBG_CMD);
            free (cmd);

            // index
            cp_wchar (dest_data, '(');
            sprintf (index_buff, "%d", watch->index);
            for (size_t i = 0; i < strlen (index_buff); i++) {
                cp_wchar (dest_data, index_buff [i]);
            }
            cp_wchar (dest_data, ')');
            cp_wchar (dest_data, ' ');

            // variable
            var_ptr = watch->var;
            while (*var_ptr != '\0') {
                cp_wchar (dest_data, *var_ptr++);
            }

            cp_wchar (dest_data, ' ');
            cp_wchar (dest_data, '=');
            cp_wchar (dest_data, ' ');

            // value
            cli_ptr = state->debugger->cli_buffer;
            if (strstr (cli_ptr, name_err) != NULL) {
                prog_ptr = (char*) none_str;
            } else {
                prog_ptr = state->debugger->program_buffer;
            }
            while (*prog_ptr != '\0') {
                cp_wchar (dest_data, *prog_ptr++);
            }

            state->debugger->cli_buffer[0] = '\0';
            state->debugger->program_buffer[0] = '\0';
            watch = watch->next;
        }
    }

    return A_OK;
}

