#include <stdlib.h>

#include "get_watchpoint_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"

static void get_watchpoint_data_gdb (state_t *state);
static void get_watchpoint_data_pdb (state_t *state);



void
get_watchpoint_data (state_t *state)
{
    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_watchpoint_data_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_watchpoint_data_pdb (state);
            break;
    }
}



static void
get_watchpoint_data_gdb (state_t *state)
{
    window_t *win;
    watchpoint_t *watch;
    const char *hex = "0x";
    char *cmd, *ptr,
         *src_ptr,
         *dest_ptr,
         *data_ptr,
         *watch_val;

    win      = state->plugins[Wat]->win;
    dest_ptr = win->buff_data->buff;
    watch    = state->watchpoints;

    win->buff_data->buff_pos = 0;
    win->buff_data->changed = true;

    // create buffer, add values
    while (watch != NULL) {

        // send watchpoint command
        cmd = concatenate_strings (3, "print ", watch->var, "\n");    
        send_command_mp (state, cmd);
        free (cmd);

        src_ptr  = state->debugger->cli_buffer;
        data_ptr = state->debugger->data_buffer;
        watch_val = watch->value;

        // index
        *dest_ptr++ = '(';
        dest_ptr += sprintf (dest_ptr, "%d", watch->index);
        *dest_ptr++ = ')';
        *dest_ptr++ = ' ';

        // variable
        dest_ptr += sprintf (dest_ptr, "%s = ", watch->var);

        if (strstr (data_ptr, "error") == NULL) {

            // skip hex value or '='
            ptr = src_ptr;
            src_ptr = strstr (src_ptr, hex);
            if (src_ptr != NULL) {
                while (*src_ptr != ' ') {
                    *dest_ptr++  = *src_ptr;
                    *watch_val++ = *src_ptr++;
                }
            } else {
                src_ptr = ptr;
                src_ptr = strstr (src_ptr, "=");
                src_ptr += 1;
            }

            src_ptr = strstr (src_ptr, " ");

            // copy value
            while (*src_ptr != '\n') {
                *dest_ptr++  = *src_ptr;
                *watch_val++ = *src_ptr++;
            }
            *watch_val = '\0';

            // remove trailing newline character
            if (*(dest_ptr - 1) == 'n' && *(dest_ptr - 2) == '\\') {
                dest_ptr -= 2;
            }

            *dest_ptr++ = '\n';
        }

        else {
            strcpy (watch->value, "none");
            strcpy (dest_ptr, "none\n");
            dest_ptr += 5;
        }

        watch = watch->next;
    }
    *dest_ptr = '\0';
}



static void
get_watchpoint_data_pdb (state_t *state)
{
    window_t *win;
    watchpoint_t *watch;
    char *cmd,
         *var_ptr,
         *cli_ptr,
          index_buff [24];
    buff_data_t *dest_data;

    const char *name_err = "*** NameError",
               *none     = "none";

    win       = state->plugins[Wat]->win;
    dest_data = win->buff_data;
    watch     = state->watchpoints;

    win->buff_data->buff_pos = 0;

    if (watch != NULL) {

        while (watch != NULL) {

            cmd = concatenate_strings (3, "p ", watch->var, "\n");    
            send_command_mp (state, cmd);
            free (cmd);

            // index
            cp_char (dest_data, '(');
            sprintf (index_buff, "%d", watch->index);
            for (size_t i = 0; i < strlen (index_buff); i++) {
                cp_char (dest_data, index_buff [i]);
            }
            cp_char (dest_data, ')');
            cp_char (dest_data, ' ');

            // variable
            var_ptr = watch->var;
            while (*var_ptr != '\0') {
                cp_char (dest_data, *var_ptr++);
            }

            cp_char (dest_data, ' ');
            cp_char (dest_data, '=');
            cp_char (dest_data, ' ');

            // value
            cli_ptr = state->debugger->cli_buffer;
            if (strstr (cli_ptr, name_err) != NULL) {
                cli_ptr = (char*) none;
            }
            while (*cli_ptr != '\0') {
                if (*cli_ptr != '\n') {
                    cp_char (dest_data, *cli_ptr);
                }
                ++cli_ptr;
            }

            cp_char (dest_data, '\n');

            watch = watch->next;
        }
    }

    else {
        cp_char (dest_data, '\0');
    }
}

