#include <ncurses.h>

#include "utilities.h"


// print int matrix
void print_int_matrix (int matrix[MAX_Y_SEGMENTS][MAX_X_SEGMENTS], int y, int x) {
   
    int col;
    int row = 5;

    for (int i = 0; i < y; i++) {
        col = 4;
        for (int j = 0; j < x; j++) {
            mvprintw (row, col, "%d", matrix [i][j]);
            col += 2;
        }
        row += 1;
    }
}
