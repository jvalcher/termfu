#include "get_breakpoint_data.h"
#include "../data.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"
#include "../plugins.h"
#include "../display_lines.h"


static void get_breakpoint_data_gdb (state_t *state);
static void get_breakpoint_data_pdb (state_t *state);



void
get_breakpoint_data (state_t *state)
{
    breakpoint_t *tmp_break,
                 *curr_break;

    // free current breakpoints
    if (state->breakpoints != NULL) {
        curr_break = state->breakpoints;
        do {
            tmp_break = curr_break->next;
            free (curr_break);
            curr_break = tmp_break;  
        } while (curr_break != NULL);
    }
    state->breakpoints = NULL;

    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            get_breakpoint_data_gdb (state);
            break;
        case (DEBUGGER_PDB):
            get_breakpoint_data_pdb (state);
            break;
    }
}



/*
    Allocate breakpoint_t struct to state->breakpoints linked list
*/
static void
allocate_breakpoint (state_t *state,
                     char    *break_buff)
{
    breakpoint_t *curr_break = state->breakpoints;

    if (state->breakpoints == NULL) {
        curr_break = (breakpoint_t*) malloc (sizeof (breakpoint_t));
        state->breakpoints = curr_break;
    } else {
        while (curr_break->next != NULL) {
            curr_break = curr_break->next;
        }
        curr_break->next = (breakpoint_t*) malloc (sizeof (breakpoint_t));
        curr_break = curr_break->next;
    }

    strncpy (curr_break->path_line, break_buff, BREAK_LEN - 1);
    curr_break->next = NULL;
}



static void
get_breakpoint_data_gdb (state_t *state)
{
    int          i;
    char        *src_ptr,
                 break_buff [BREAK_LEN];
    buff_data_t *dest_buff;
    
    const char *key_number   = "number=\"",
               *key_orig_loc = "original-location=\"",
               *key_nr_rows  = "nr_rows=\"";

    src_ptr   = state->debugger->data_buffer;
    dest_buff = state->plugins[Brk]->win->buff_data;

    insert_output_start_marker (state);
    send_command (state, "-break-info\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    if (strstr (src_ptr, "error") == NULL) {

        dest_buff->buff_pos = 0;

        // get number of breakpoints
        src_ptr = strstr (src_ptr, key_nr_rows);
        src_ptr += strlen (key_nr_rows);
        i = 0;
        while (*src_ptr != '\"') {
            break_buff[i++] = *src_ptr++;
        }
        break_buff[i] = '\0';

        if (break_buff[0] > '0') {

            // get breakpoint number
            while ((src_ptr = strstr (src_ptr, key_number)) != NULL && break_buff[0] > '0') {

                cp_char (dest_buff, '(');

                src_ptr += strlen (key_number);
                while (*src_ptr != '\"') {
                    cp_char (dest_buff, *src_ptr++);
                }

                cp_char (dest_buff, ')');
                cp_char (dest_buff, ' ');

                // get file:line
                i = 0;
                src_ptr = strstr (src_ptr, key_orig_loc);
                src_ptr += strlen (key_orig_loc);
                while (*src_ptr != '\"') {
                    break_buff [i++] = *src_ptr;
                    cp_char (dest_buff, *src_ptr++);
                }
                break_buff [i] = '\0';

                allocate_breakpoint (state, break_buff);

                cp_char (dest_buff, '\n');
            }
        }

        else {
            cp_char (dest_buff, '\0');
        }

        dest_buff->changed = true;
    }
}



static void
get_breakpoint_data_pdb (state_t *state)
{
    int          i;
    window_t    *win;
    char        *src_ptr,
                 break_buff [BREAK_LEN];
    buff_data_t *dest_buff;

    const char  *at_str    = "at ",
                *break_str = "breakpoint",
                *hit_str   = "\tbreakpoint already hit",
                *num_str   = "Num ";

    win       = state->plugins[Brk]->win;
    src_ptr   = state->debugger->cli_buffer;
    dest_buff = win->buff_data;

    dest_buff->buff_pos = 0;
    dest_buff->changed = true;

    send_command_mp (state, "break\n");

    if (strstr (src_ptr, break_str) != NULL) {

        while (*src_ptr != '\0') {

            // skip table header line
            if (strncmp (src_ptr, num_str, strlen (num_str)) == 0 ||
                strncmp (src_ptr, hit_str, strlen (hit_str)) == 0   ) {
                while (*src_ptr != '\n') {
                    ++src_ptr;
                }
            }

            else if (*src_ptr != '\n' && *src_ptr != '\'') {

                // index
                cp_char (dest_buff, '(');
                do {
                    cp_char (dest_buff, *src_ptr++);
                } while (*src_ptr != ' ');

                cp_char (dest_buff, ')');
                cp_char (dest_buff, ' ');

                // path:line
                i = 0;
                src_ptr = strstr (src_ptr, at_str);
                src_ptr += strlen (at_str);
                while (*src_ptr != '\n') {
                    break_buff [i++] = *src_ptr;
                    cp_char (dest_buff, *src_ptr++);
                }
                break_buff [i] = '\0';

                allocate_breakpoint (state, break_buff);

                cp_char (dest_buff, '\n');
            }

            else {
                ++src_ptr;
            }
        }

        // remove final newline
        if (dest_buff->buff [strlen (dest_buff->buff) - 1] == '\n') {
            dest_buff->buff [strlen (dest_buff->buff) - 1] = '\0';
        }
    } 

    else {
        dest_buff->buff[0] = '\0';
    }

    win->buff_data->changed = true;;
}
