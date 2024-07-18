
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>


int main (void)
{
    initscr();
    int nc_rows = getmaxy (stdscr);
    int nc_cols = getmaxx (stdscr);
    endwin ();

    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl");
        return 1;
    }

    printf ("nc rows: %d\n", nc_rows);
    printf ("nc cols: %d\n", nc_cols);
    printf ("sc rows: %d\n", w.ws_row);
    printf ("sc rows: %d\n", w.ws_col);


    return 0;
}
