#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <termio.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdarg.h>

#include "utilities.h"
#include "data.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"



/*
   Close ncurses
*/
void clean_up (void)
{
    // Ncurses
    curs_set (1);
    endwin ();
}



/*
    Concatenate strings
    --------
    - must free returned string
*/
char*
concatenate_strings (int num_strs, ...)
{
    char     buffer [PATH_MAX_LEN] = {0},
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



/*
    TODO: integrate concatenate_strings(), automatically detect num_strs
*/
void
send_command (state_t *state,
              int num_strs,
              ...)
{
    char     buffer [CMD_MAX_LEN] = {0},
            *sub_cmd;
    va_list  cmds;

    // create command string
    va_start (cmds, num_strs);
    for (int i = 0; i < num_strs; i++) {
        sub_cmd = va_arg (cmds, char*);
        strncat (buffer, sub_cmd, sizeof(buffer) - strlen(buffer) - 1);
    }
    va_end (cmds);

    // send command
    insert_output_start_marker (state);
    size_t bytes = write (state->debugger->stdin_pipe, buffer, strlen (buffer));
    if (bytes == 0) {
        pfeme ("No bytes written");
    }
    insert_output_end_marker (state);

    parse_debugger_output (state);
}



int
getkey (void)
{
#ifndef DEBUG
        return getch();
#endif

#ifdef DEBUG

    int key,
        stdout_fd,
        dev_null_fd;

    static struct termios oldt, newt;
    dev_null_fd = open ("/dev/null", O_WRONLY);
    stdout_fd   = dup (STDOUT_FILENO);

    // disable need to hit enter after key press
    tcgetattr (STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);          
    tcsetattr (STDIN_FILENO, TCSANOW, &newt);
    dup2 (dev_null_fd, STDOUT_FILENO);
        //
    key = getchar();
        //
    dup2 (stdout_fd, STDOUT_FILENO);
    tcsetattr (STDIN_FILENO, TCSANOW, &oldt);
    close (dev_null_fd);

    return key;

#endif
}



/*
    Set Ncurses attribute with variable instead of constant
*/
void set_nc_attribute (WINDOW* win, int attr)
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
void unset_nc_attribute (WINDOW* win, int attr)
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
copy_string_buffer (char *src_buff,
                    char *dest_buff)
{
    char *sb = src_buff,
         *db = dest_buff;

    while (*sb != '\0') {
        *db++ = *sb++;
    }
    *db = *sb;
}



