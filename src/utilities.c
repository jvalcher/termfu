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

FILE *debug_out_ptr = NULL;



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



void clean_up (void)
{
    // exit Ncurses
    curs_set (1);
    endwin ();

    // close DEBUG_OUT_FILE
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



void
cp_char (buff_data_t *dest_buff_data,
         char ch)
{
    dest_buff_data->buff[dest_buff_data->buff_pos] = ch;
    dest_buff_data->buff[dest_buff_data->buff_pos + 1] = '\0';

    if (dest_buff_data->buff_pos < dest_buff_data->buff_len - 2) {
        ++dest_buff_data->buff_pos;
    } else {
        dest_buff_data->buff_pos = 0;
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
    }

    buff[*pos] = ch;
    buff[*pos + 1] = '\0';

    if (*pos < *len - 2) {
        *pos += 1;
    } 

    else {

        pfem ("Character copy buffer overflow");

        switch (type) {
            case PATH:
                peme ("win->src_file_data->path : %s", buff);
                break;
            case ADDR:
                peme ("win->src_file_data->addr : %s", buff);
                break;
            case FUNC:
                peme ("win->src_file_data->func : %s", buff);
                break;
        }
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



/*
    Copy character into debugger buffer
*/
void
cp_dchar (debugger_t *debugger, char ch, int buff_index)
{
    char *buff;
    int  *len,
         *pos;

    switch (buff_index) {
        case FORMAT_BUF:
            buff =  debugger->format_buffer;
            len  = &debugger->format_len;
            pos  = &debugger->format_pos;
            break;
        case DATA_BUF:
            buff =  debugger->data_buffer;
            len  = &debugger->data_len;
            pos  = &debugger->data_pos;
            break;
        case CLI_BUF:
            buff =  debugger->cli_buffer;
            len  = &debugger->cli_len;
            pos  = &debugger->cli_pos;
            break;
        case PROGRAM_BUF:
            buff =  debugger->program_buffer;
            len  = &debugger->program_len;
            pos  = &debugger->program_pos;
            break;
        case ASYNC_BUF:
            buff =  debugger->async_buffer;
            len  = &debugger->async_len;
            pos  = &debugger->async_pos;
            break;
    }

    buff [*pos] = ch;
    buff [*pos + 1] = '\0';

    if (*pos < *len - 2) {
        *pos += 1;
    } else {
        *pos = 0;
    }
}



