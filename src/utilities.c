#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <termio.h>
#include <fcntl.h>
#include <stdarg.h>

#include "utilities.h"
#include "data.h"
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
create_path (int num_strs, ...)
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



void
send_command (state_t *state,
              int num_cmds,
              ...)
{
    char     buffer [CMD_MAX_LEN] = {0},
            *sub_cmd;
    va_list  cmds;

    // create command string
    va_start (cmds, num_cmds);
    for (int i = 0; i < num_cmds; i++) {
        sub_cmd = va_arg (cmds, char*);
        strncat (buffer, sub_cmd, sizeof(buffer) - strlen(buffer) - 1);
    }
    va_end (cmds);

    // send command
    size_t bytes = write (state->debugger->stdin_pipe, buffer, strlen (buffer));
    if (bytes == 0) {
        pfeme ("No bytes written");
    }
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
    Write to FIFO pipe
*/
void
write_to_pipe (int n,
               char **strings)
{
    char buffer [256];
    for (int i = 0; i < n; i++) {
        strncat (buffer, strings[i], sizeof (buffer) - 1);
    }
    printf ("%s\n", buffer);
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



/*
    Print colored string in Ncurses window
    ---------
    - Setting an Ncurses color (apparently) requires a constant color value
    - This function allows the use of a color variable from data.h

    - Usage:
        int my_color = MAGENTA_BLACK;
        print_nc_str (my_color, win, y, x, "%s", msg);
*/
void print_nc_str (int     color, 
                   WINDOW *win,
                   int     row,
                   int     col, 
                   char   *str)
{
    set_nc_attribute (win, color);
    mvwprintw (win, row, col, "%s", str);
    unset_nc_attribute (win, color);
    wrefresh  (win);
}



