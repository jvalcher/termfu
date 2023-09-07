
/*
   gdb-tuiffic
 */

#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "ncurses_utilities.h"

#define VERSION         "0.0.1"
#define CMD_LEN         50


int main (void) 
{
    // set up window
    initscr();
    render_window();

    // testing

    // create windows
    // newwin(rows, cols, y_org, x_org);
    int h, w;
    getmaxyx(stdscr, h, w);
    WINDOW *main = newwin(10,10,0,0);
    wrefresh(main);

    refresh();
    //getch();
    endwin();

    return 0;
}

/*

    // get row, column
    int x, y;
    getmaxyx(stdscr, y, x);
    printw("%d rows, %d columns\n", y, x);
    printw("%d rows, %d columns\n", LINES, COLS);
    mvaddstr(3, 0, "Test");
    refresh();

    // get cursor position
    getyx(stdscr, y, x);

    // enter command
    char command[CMD_LEN];
    getnstr(command, CMD_LEN - 1);
    move(2, 0);

    // insert line
    addstr("Hello\n");
    addstr("there\n");
    move(1, 0);
    insertln();
    addstr("Insert\n");

    // file.c
    char *filename = "file";
    char *ext = ".c";
    int len = strlen(filename);
    addstr(filename);
    move(1,len);
    insstr(ext);

    // buffer
    char buff[101];
    addstr("Input: ");
    refresh();
    napms(5000);
    addstr("\nFlushing buffer...\n");
    flushinp();
    getnstr(buff, 100);
    printw("You typed: %s\n", buff);    // no output
*/

