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
            if (curr_break) {
                free (curr_break);
            }
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
                     char    *index_buff,
                     char    *file_buff,
                     char    *line_buff)
{
    breakpoint_t *curr_break = state->breakpoints;

    // allocate
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
    curr_break->next = NULL;

    // add data
    memcpy (curr_break->index, index_buff, INDEX_BUFF_LEN - 1);
    curr_break->index[INDEX_BUFF_LEN-1] = '\0';
    memcpy (curr_break->path, file_buff, BREAK_PATH_LEN - 1);
    curr_break->path[BREAK_PATH_LEN-1] = '\0';
    memcpy (curr_break->line, line_buff, BREAK_LINE_LEN - 1);
    curr_break->line[BREAK_LINE_LEN-1] = '\0';


    return A_OK;

alloc_break_err:
    pemr ("index: \"%s\"; path: \"%s\"; line: \"%s\"", index_buff, file_buff, line_buff);
}



static int
get_breakpoint_data_gdb (state_t *state)
{
    int          i, ret;
    char        *src_ptr,
                 file_buff   [BREAK_PATH_LEN],
                 line_buff   [BREAK_LINE_LEN],
                 index_buff  [INDEX_BUFF_LEN];
    buff_data_t *dest_buff;
    
    const char *key_number   = "number=\"",
               *key_file     = "file=\"",
               *key_line     = "line=\"",
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
        i = 0;
        src_ptr = strstr (src_ptr, key_nr_rows);
        src_ptr += strlen (key_nr_rows);
        while (*src_ptr != '\"') {
            file_buff[i++] = *src_ptr++;
        }
        file_buff[i] = '\0';

        if (file_buff[0] > '0') {

            // "number=..."
            while ((src_ptr = strstr (src_ptr, key_number)) != NULL) {
                
                file_buff [0]  = '\0';
                line_buff [0]  = '\0';
                index_buff [0] = '\0';

                cp_wchar (dest_buff, '(');

                // index
                i = 0;
                src_ptr += strlen (key_number);
                while (*src_ptr != '\"') {
                    index_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                }
                index_buff [i] = '\0';

                cp_wchar (dest_buff, ')');
                cp_wchar (dest_buff, ' ');

                // file
                i = 0;
                src_ptr  = strstr (src_ptr, key_file);
                src_ptr += strlen (key_file);
                while (*src_ptr != '\"') {
                    file_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                }
                file_buff [i] = '\0';

                // ':'
                cp_wchar (dest_buff, ':');

                // line
                i = 0;
                src_ptr  = strstr (src_ptr, key_line);
                src_ptr += strlen (key_line);
                while (*src_ptr != '\"') {
                    line_buff [i++] = *src_ptr;
                    cp_wchar (dest_buff, *src_ptr++);
                }
                line_buff [i] = '\0';

                // allocate breakpoint_t
                ret = allocate_breakpoint (state, index_buff, file_buff, line_buff);
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
                 path_buff  [BREAK_PATH_LEN*2],
                *file_ptr,
                 index_buff      [INDEX_BUFF_LEN],
                 break_line_buff [BREAK_LINE_LEN];
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
                file_ptr = basename (path_buff);

                // file
                i = 0;
                while (file_ptr [i] != '\0') {
                    cp_wchar (dest_buff, file_ptr [i++]);
                }

                // :
                i = 0;
                if (*src_ptr == ':') {
                    cp_wchar (dest_buff, *src_ptr++);

                    // line
                    while (*src_ptr != '\n') {
                        break_line_buff [i++] = *src_ptr;
                        cp_wchar (dest_buff, *src_ptr++);
                    }
                }
                break_line_buff [i] = '\0';

                ret = allocate_breakpoint (state, index_buff, file_ptr, break_line_buff);
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
