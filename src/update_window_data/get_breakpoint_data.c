#include <libgen.h>

#include "get_breakpoint_data.h"
#include "../data.h"
#include "../utilities.h"
#include "../plugins.h"

static int get_breakpoint_data_gdb (state_t *state);
static int get_breakpoint_data_pdb (state_t *state);

#define INDEX_BUFF_LEN  8



int
get_breakpoint_data (state_t *state)
{
    int ret;
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
            ret = get_breakpoint_data_gdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get breakpoint data (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            ret = get_breakpoint_data_pdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get breakpoint data (PDB)");
            }
            break;
    }

    return A_OK;
}



/*
    Allocate breakpoint_t struct to state->breakpoints linked list
*/
static int
allocate_breakpoint (state_t *state,
                     char    *break_buff,
                     char    *index_buff)
{
    breakpoint_t *curr_break = state->breakpoints;

    if (state->breakpoints == NULL) {
        if ((curr_break = (breakpoint_t*) malloc (sizeof (breakpoint_t))) == NULL) {
            pfem ("malloc error (first): %s", strerror (errno));
            goto alloc_break_err;
        }
        state->breakpoints = curr_break;
    } else {
        while (curr_break->next != NULL) {
            curr_break = curr_break->next;
        }
        if ((curr_break->next = (breakpoint_t*) malloc (sizeof (breakpoint_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            goto alloc_break_err;
        }
        curr_break = curr_break->next;
    }

    memcpy (curr_break->index, index_buff, INDEX_BUFF_LEN - 1);
    curr_break->index[INDEX_BUFF_LEN-1] = '\0';

    memcpy (curr_break->path_line, break_buff, BREAK_LEN - 1);
    curr_break->path_line[BREAK_LEN-1] = '\0';

    curr_break->next = NULL;

    return A_OK;

alloc_break_err:
    pemr ("index: \"%s\"; breakpoint: \"%s\"", index_buff, break_buff);
}



static int
get_breakpoint_data_gdb (state_t *state)
{
    int          i, ret;
    char        *src_ptr,
                 break_buff [BREAK_LEN],
                 index_buff [INDEX_BUFF_LEN];
    buff_data_t *dest_buff;
    
    const char *key_number   = "number=\"",
               *key_orig_loc = "original-location=\"",
               *key_nr_rows  = "nr_rows=\"";

    src_ptr   = state->debugger->data_buffer;
    dest_buff = state->plugins[Brk]->win->buff_data;

    ret = send_command_mp (state, "-break-info\n");
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }

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
            while ((src_ptr = strstr (src_ptr, key_number)) != NULL) {

                cp_wchar (dest_buff, '(');

                i = 0;
                src_ptr += strlen (key_number);
                while (*src_ptr != '\"') {
                    index_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                }
                index_buff [i] = '\0';

                cp_wchar (dest_buff, ')');
                cp_wchar (dest_buff, ' ');

                // get file:line
                i = 0;
                src_ptr = strstr (src_ptr, key_orig_loc);
                src_ptr += strlen (key_orig_loc);
                while (*src_ptr != '\"') {
                    break_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                }
                break_buff [i] = '\0';

                ret = allocate_breakpoint (state, break_buff, index_buff);
                if (ret == FAIL) {
                    pfemr ("Failed to allocate breakpoint");
                }

                cp_wchar (dest_buff, '\n');
            }
        }

        else {
            cp_wchar (dest_buff, '\0');
        }

        dest_buff->changed = true;
    }

    return A_OK;
}



static int
get_breakpoint_data_pdb (state_t *state)
{
    int          i, ret;
    window_t    *win;
    char        *src_ptr,
                 path_buff  [BREAK_LEN*2],
                *basename_ptr,
                 break_buff [BREAK_LEN],
                 index_buff [INDEX_BUFF_LEN];
    buff_data_t *dest_buff;

    const char  *at_str    = "at ",
                *break_str = "breakpoint",
                *hit_str   = "\tbreakpoint already hit",
                *del_str   = "Deleted breakpoint",
                *num_str   = "Num ";

    win       = state->plugins[Brk]->win;
    src_ptr   = state->debugger->program_buffer;
    dest_buff = win->buff_data;

    dest_buff->buff_pos = 0;
    dest_buff->changed = true;

    ret = send_command_mp (state, "break\n");
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }

    if (strncmp (src_ptr, del_str, strlen (del_str)) == 0) {
        goto skip_Brk_parse_pdb;
    }

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

                cp_wchar (dest_buff, '(');

                // index
                i = 0;
                do {
                    index_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                } while (*src_ptr != ' ');
                index_buff [i] = '\0';

                cp_wchar (dest_buff, ')');
                cp_wchar (dest_buff, ' ');

                // path
                i = 0;
                src_ptr = strstr (src_ptr, at_str);
                src_ptr += strlen (at_str);
                while (*src_ptr != ':') {
                    path_buff [i++] = *src_ptr++;
                }
                path_buff[i] = '\0';

                // path -> basename
                basename_ptr = basename (path_buff);

                // filename
                i = 0;
                while (basename_ptr [i] != '\0') {
                    break_buff [i] = basename_ptr [i];
                    cp_wchar (dest_buff, break_buff [i++]);
                }

                // :line
                while (*src_ptr != '\n') {
                    break_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                }
                break_buff [i] = '\0';

                ret = allocate_breakpoint (state, break_buff, index_buff);
                if (ret == FAIL) {
                    pfemr ("Failed to allocate breakpoint");
                }

                cp_wchar (dest_buff, '\n');
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
skip_Brk_parse_pdb:
        cp_wchar (dest_buff, '\0');
    }

    state->debugger->program_buffer[0]  = '\0';
    win->buff_data->changed = true;;

    return A_OK;
}
