#include <ncursesw/ncurses.h>

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
