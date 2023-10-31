#include <ncurses.h>
#include <stdarg.h>

#include "utilities.h"


// print int matrix
void print_int_matrix ( char *label,
                        int start_row, 
                        int matrix[MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS], 
                        int y, 
                        int x) 
{
    int col = 4;
    int row = start_row;

    mvprintw (row, col, "%s", label);
    row += 1;
    mvprintw (row, col, "--------");
    row += 1;

    for (int i = 0; i < y; i++) {
        col = 4;
        for (int j = 0; j < x; j++) {
            mvprintw (row, col, "%d", matrix [i][j]);
            col += 4;
        }
        row += 1;
    }
}



/*
    Print bold, colored Ncurses string with mvwprintw()
*/
void mv_print_title (int color, 
                     WINDOW* win,
                     int row,
                     int col, 
                     char *str)
{
    // set color attribute
    switch (color) {
        case RED_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(RED_BLACK));
            break;
        case GREEN_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(GREEN_BLACK));
            break;
        case YELLOW_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(YELLOW_BLACK));
            break;
        case BLUE_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(BLUE_BLACK));
            break;
        case MAGENTA_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(MAGENTA_BLACK));
            break;
        case CYAN_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(CYAN_BLACK));
            break;
        case WHITE_BLACK:
            wattron (win, A_BOLD | COLOR_PAIR(WHITE_BLACK));
            break;
    }

    // print string
    mvwprintw (win, row, col, "%s", str);

    // reset attributes
    wattrset (win, A_NORMAL);
}
