#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <errno.h>

#include "utilities.h"
#include "data.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"
#include "persist_data.h"

FILE *debug_out_ptr = NULL;
state_t *state_ptr = NULL;



void
logd (const char *formatted_string, ...)
{
    if (debug_out_ptr == NULL) {
        if ((debug_out_ptr = fopen (DEBUG_OUT_FILE, "w")) == NULL) {
            pfem ("fopen error: %s", strerror (errno));
            peme ("Failed to open debug out file \"%s\"", DEBUG_OUT_FILE);
        }
    }

    va_list args;
    va_start (args, formatted_string);
    vfprintf (debug_out_ptr, formatted_string, args);
    va_end (args);
}



void
clean_up (void)
{
    curs_set (1);
    endwin ();

    if (persist_data (state_ptr) == RET_FAIL) {
        pfem ("Failed to persist data");
    }

    if (debug_out_ptr != NULL) {
        fclose (debug_out_ptr);
    }
}



char*
concatenate_strings (int num_strs, ...)
{
    char     buffer [FILE_PATH_LEN] = {0},
            *sub_str,
            *str;
    va_list  strs;

    // create path string
    va_start (strs, num_strs);
    for (int i = 0; i < num_strs; i++) {
        sub_str = va_arg (strs, char*);
        strncat (buffer, sub_str, sizeof(buffer) - strlen(buffer) - 1);
    }
    va_end (strs);

    // allocate
    if ((str = (char*) malloc (strlen (buffer) + 1)) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        peme ("Failed to concatenate strings \"%s\"", buffer);
    }

    strcpy (str, buffer);

    return str;
}



int
send_command (state_t *state,
              char    *command)
{
    if (write (state->debugger->stdin_pipe, command, strlen (command)) == -1) {
        pfem ("write error: %s", strerror (errno));
        pemr ("Command: \"%s\"", command);
    }
    return RET_OK;
}



int
send_command_mp (state_t *state,
                 char *command)
{
    if (insert_output_start_marker (state) == RET_FAIL) {
        pfemr ("Failed to send command start marker");
    }

    if (write (state->debugger->stdin_pipe, command, strlen (command)) == -1) {
        pfem ("write error: %s", strerror (errno));
        pemr ("Command: \"%s\"", command);
    }

    if (insert_output_end_marker (state) == RET_FAIL) {
        pfemr ("Failed to send command end marker");
    }

    parse_debugger_output (state);

    return RET_OK;
}



/*
    Set Ncurses attribute with variable instead of constant
*/
int
set_nc_attribute (WINDOW* win,
                  int attr)
{
    switch (attr) {

        // colors (data.h)
        case RED_BLACK:
            wattron (win, COLOR_PAIR(RED_BLACK));
            break;
        case GREEN_BLACK:
            wattron (win, COLOR_PAIR(GREEN_BLACK));
            break;
        case YELLOW_BLACK:
            wattron (win, COLOR_PAIR(YELLOW_BLACK));
            break;
        case BLUE_BLACK:
            wattron (win, COLOR_PAIR(BLUE_BLACK));
            break;
        case MAGENTA_BLACK:
            wattron (win, COLOR_PAIR(MAGENTA_BLACK));
            break;
        case CYAN_BLACK:
            wattron (win, COLOR_PAIR(CYAN_BLACK));
            break;
        case WHITE_BLACK:
            wattron (win, COLOR_PAIR(WHITE_BLACK));
            break;

        // other
        case A_BOLD:
            wattron (win, A_BOLD);
            break;
        case A_UNDERLINE:
            wattron (win, A_UNDERLINE);
            break;

        default:
            pfemr ("Unsupported attribute \"%d\"", attr);
    }

    return RET_OK;
}



/*
    Unset Ncurses attribute with variable instead of constant
*/
int
unset_nc_attribute (WINDOW* win,
                    int attr)
{
    switch (attr) {

        // colors (data.h)
        case RED_BLACK:
            wattroff (win, COLOR_PAIR(RED_BLACK));
            break;
        case GREEN_BLACK:
            wattroff (win, COLOR_PAIR(GREEN_BLACK));
            break;
        case YELLOW_BLACK:
            wattroff (win, COLOR_PAIR(YELLOW_BLACK));
            break;
        case BLUE_BLACK:
            wattroff (win, COLOR_PAIR(BLUE_BLACK));
            break;
        case MAGENTA_BLACK:
            wattroff (win, COLOR_PAIR(MAGENTA_BLACK));
            break;
        case CYAN_BLACK:
            wattroff (win, COLOR_PAIR(CYAN_BLACK));
            break;
        case WHITE_BLACK:
            wattroff (win, COLOR_PAIR(WHITE_BLACK));
            break;
        case WHITE_BLUE:
            wattroff (win, COLOR_PAIR(WHITE_BLUE));
            break;

        // other
        case A_BOLD:
            wattroff (win, A_BOLD);
            break;
        case A_UNDERLINE:
            wattroff (win, A_UNDERLINE);
            break;

        default:
            pfemr ("Unsupported attribute \"%d\"", attr);
    }

    return RET_OK;
}



bool
find_window_string (WINDOW *window,
                    char   *string,
                    int    *y,
                    int    *x)
{
    int  i, j, 
         m, n,
         ch,
         rows, cols;
    size_t si = 0;
    bool found = false;

    // get number of rows, columns
    getmaxyx (window, rows, cols);

    // find string
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            ch = mvwinch (window, i, j);

            if ((char) ch == string [si]) {
                if (si == 0) {
                    m = i;
                    n = j;
                }
                si += 1;
                if (si == strlen (string)) {
                    found = true;
                    break;
                }
            } else {
                si = 0;
            }
        }
        if (found) {
            break;
        }
    }

    if (found) {
        *y = m;
        *x = n;
        return true;
    } else {
        return false;
    }
}



bool
file_was_updated (time_t file_mtime,
                  char *file_path)
{
    struct stat file_stat;

    if (stat (file_path, &file_stat) != 0) {
        return false;
    }

    return file_mtime < file_stat.st_mtim.tv_sec;
}



void
set_state_ptr (state_t *state)
{
    state_ptr = state;
}



void
cp_char (buff_data_t *dest_buff_data,
         char ch)
{
    char *tmp;

    dest_buff_data->buff[dest_buff_data->buff_pos] = ch;
    dest_buff_data->buff[dest_buff_data->buff_pos + 1] = '\0';

    if (dest_buff_data->buff_pos < dest_buff_data->buff_len - 1) {
        ++dest_buff_data->buff_pos;
    } 

    else {

        // double buffer size
        if (dest_buff_data->times_doubled < MAX_DOUBLE_TIMES) {

            dest_buff_data->buff_len *= 2;
            ++dest_buff_data->times_doubled;

            tmp = (char*) realloc (dest_buff_data->buff, sizeof (char) * dest_buff_data->buff_len);
            if (tmp == NULL) {
                pfem ("realloc error: %s", strerror (errno));
                peme ("Failed to reallocate window buffer (code: %s, times doubled: %d, buff size: %d)",
                            dest_buff_data->code, dest_buff_data->times_doubled, dest_buff_data->buff_len);
            }

            dest_buff_data->buff = tmp;
        }

        // or loop back to buffer start
        else {
            dest_buff_data->buff_pos = 0;
        }
    }
}



void
cp_fchar (src_file_data_t *dest_file_data,
          char ch,
          int type)
{
    char *buff;
    int  *len,
         *pos;

    switch (type) {
        case PATH:
            buff = dest_file_data->path;
            len = &dest_file_data->path_len;
            pos = &dest_file_data->path_pos;
            break;
        case ADDR:
            buff = dest_file_data->addr;
            len = &dest_file_data->addr_len;
            pos = &dest_file_data->addr_pos;
            break;
        case FUNC:
            buff = dest_file_data->func;
            len = &dest_file_data->func_len;
            pos = &dest_file_data->func_pos;
            break;
        default:
            pfeme ("Unrecognized buffer type \"%d\"", type);
    }

    buff[*pos] = ch;
    buff[*pos + 1] = '\0';

    if (*pos < *len - 1) {
        *pos += 1;
    } 

    else {

        pfem ("Buffer overflow");

        switch (type) {
            case PATH:
                peme ("win->src_file_data->path : \"%s...\"", buff);
                break;
            case ADDR:
                peme ("win->src_file_data->addr : \"%s...\"", buff);
                break;
            case FUNC:
                peme ("win->src_file_data->func : \"%s...\"", buff);
                break;
        }
    }
}



void
cp_dchar (debugger_t *debugger,
          char ch,
          int buff_index)
{
    int  *len,
         *pos,
         *doubled;
    char *buff,
         *tmp,
         *title,
         *form_title  = "format",
         *data_title  = "data",
         *cli_title   = "cli",
         *prog_title  = "program",
         *async_title = "async";


    switch (buff_index) {
        case FORMAT_BUF:
            title   =  form_title;
            buff    =  debugger->format_buffer;
            len     = &debugger->format_len;
            pos     = &debugger->format_pos;
            doubled = &debugger->format_times_doubled;
            break;
        case DATA_BUF:
            title   =  data_title;
            buff    =  debugger->data_buffer;
            len     = &debugger->data_len;
            pos     = &debugger->data_pos;
            doubled = &debugger->data_times_doubled;
            break;
        case CLI_BUF:
            title   =  cli_title;
            buff    =  debugger->cli_buffer;
            len     = &debugger->cli_len;
            pos     = &debugger->cli_pos;
            doubled = &debugger->cli_times_doubled;
            break;
        case PROGRAM_BUF:
            title   =  prog_title;
            buff    =  debugger->program_buffer;
            len     = &debugger->program_len;
            pos     = &debugger->program_pos;
            doubled = &debugger->program_times_doubled;
            break;
        case ASYNC_BUF:
            title   =  async_title;
            buff    =  debugger->async_buffer;
            len     = &debugger->async_len;
            pos     = &debugger->async_pos;
            doubled = &debugger->async_times_doubled;
            break;
        default:
            pfeme ("Unrecognized debugger buffer index \"%d\"", buff_index);
    }

    buff [*pos] = ch;
    buff [*pos + 1] = '\0';

    if (*pos < *len - 1) {
        *pos += 1;
    } 

    else {

        // double buffer size
        if (*doubled < MAX_DOUBLE_TIMES) {

            *len     *= 2;
            *doubled += 1;

            tmp = (char*) realloc (buff, sizeof (char) * *len);
            if (tmp == NULL) {
                pfem ("realloc error: %s", strerror (errno));
                pem  ("Failed to reallocate \"%s\" buffer", title);
                peme ("Buffer size: %d, position: %d, Times doubled: %d)", *len, *pos, *doubled);
            }

            switch (buff_index) {
                case FORMAT_BUF:
                    debugger->format_buffer = tmp;
                    break;
                case DATA_BUF:
                    debugger->data_buffer = tmp;
                    break;
                case CLI_BUF:
                    debugger->cli_buffer = tmp;
                    break;
                case PROGRAM_BUF:
                    debugger->program_buffer = tmp;
                    break;
                case ASYNC_BUF:
                    debugger->async_buffer = tmp;
                    break;
            }
        }

        // or loop back to buffer start
        else {
            *pos = 0;
        }
    }
}

