#include <ncurses.h>
#include <unistd.h>

#include "utilities.h"
#include "data.h"



/*
   Close ncurses
*/
void close_ncurses (void)
{
#ifndef DEBUG
    curs_set (1);
    endwin ();
#endif
}



/*
    Clear file, open for appending by its path
*/
FILE *clear_and_open_file_for_append (char *path)
{
    FILE *fp = fopen (path, "w");
    fclose (fp);
    return fopen (path, "a");
}



char *get_code_path (char *code, plugin_t *plugins)
{
    do {
        if (strcmp (code, plugins->code) == 0)
            break;
        plugins = plugins->next;
    } while (plugins != NULL);
    return plugins->window->out_file_path;
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



