#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>

#include "utilities.h"
#include "data.h"

FILE *debug_out_ptr = NULL;



/*
   Log debug output to file
*/
void
logd (const char *formatted_string, ...)
{
    if (debug_out_ptr == NULL) {
        debug_out_ptr = fopen (DEBUG_OUT_FILE, "w");
        if (debug_out_ptr == NULL) {
            pfeme ("Unable to open debug output file\n");
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
    str = (char*) malloc (strlen (buffer) + 1);
    if (str == NULL) {
        fprintf (stderr, "String malloc failed");
        exit (EXIT_FAILURE);
    }
    strcpy (str, buffer);

    return str;
}



void
send_command (state_t *state,
              char *command)
{
    size_t bytes = write (state->debugger->stdin_pipe, command, strlen (command));
    if (bytes == 0) {
        pfeme ("No bytes written to debugger process");
    }
}



/*
    Set Ncurses attribute with variable instead of constant
*/
void
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
    }
}



/*
    Unset Ncurses attribute with variable instead of constant
*/
void
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
    }
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
cp_fchar (file_data_t *dest_file_data,
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
                peme ("win->file_data->path : %s", buff);
                break;
            case ADDR:
                peme ("win->file_data->addr : %s", buff);
                break;
            case FUNC:
                peme ("win->file_data->func : %s", buff);
                break;
        }
    }
}



FILE*
open_config_file (void)
{
    FILE *file = fopen (CONFIG_FILE, "r");
    if (file == NULL) {
        pfeme ("Unable to find config file \"./%s\"\n", CONFIG_FILE);
    }
    return file;
}



/*
    Get category and label
    -------
        [ <categ> : <label> ]
*/
void
get_category_and_label (FILE *file,
                        char *category,
                        char *label)
{
    int i,
        ch = 0;

    do {
        // category
        i = 0;
        while (((ch = fgetc (file)) != ':'  &&
                                 ch != ']') &&
                                  i <  MAX_CONFIG_CATEG_LEN - 1) {
            if (ch != ' ')
                category [i++] = ch;
        }
        category [i] = '\0'; 

        // if no label, break
        if (ch == ']') {
            label = "\0";
            break;
        }

        // label
        i = 0;
        while ((ch = fgetc (file)) != ']' &&
                    i < MAX_CONFIG_LABEL_LEN - 1) {
            label [i++] = ch;
        }
        label [i] = '\0';

    } while (ch != ']');
}



