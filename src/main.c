
/*
   gdb-tuiffic
 */

#include <stdio.h>
#include <string.h>
#include <ncurses.h>

#include "colors.h"
#include "apply_config.h"
#include "render_screen.h"

#define VERSION         "0.0.1"
#define CMD_LEN         50
#define RUNNING         true


int main (void) 
{
    // initialize Ncurses
    initscr();

    // initialize font colors (colors.h)
    create_colors ();

    // parse external configuration file (parse_config.h)
    struct layouts *layouts = NULL;
    apply_config (layouts);

    //
    //  Main loop
    //  ---------
    //  q   - quit
    //
    int ch;
    bool running = true;
    while (running) {

        render_screen (layouts);    // render_screen.h

        // read keypress
        noecho();
        ch = getch();
        echo();

        // actions
        nodelay(stdscr, true);

        // quit
        if (ch == 'q')
            break;

        nodelay(stdscr, false);
    }

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

    // refresh()
    // wrefresh()
    // touchwin(stdscr) + refresh()

    // delwin(win)
*/

