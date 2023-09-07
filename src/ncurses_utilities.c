
/*
    Ncurses rendering utilities

*/


#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#include "nc_utilities.h"

static void display_title();
static void enable_colors();
static void center_str (int row, char *str);

/*
    render window
*/
void render_window()
{
    enable_colors();
    display_title();
}


/*
   Enable colors if possible
*/
static void enable_colors()
{
    // check current terminal's colors with:  make colors
    if (has_colors()) {

        start_color();

        // create font color pairs for black background
        init_pair(RED, COLOR_RED, 0);
        init_pair(GREEN, COLOR_GREEN, 0);
        init_pair(YELLOW, COLOR_YELLOW, 0);
        init_pair(BLUE, COLOR_BLUE, 0);
        init_pair(MAGENTA, COLOR_MAGENTA, 0);
        init_pair(CYAN, COLOR_CYAN, 0);
        init_pair(WHITE, COLOR_WHITE, 0);
    }
}


/* 
   Display program title
*/
static void display_title()
{
    move(0, 10);
    attron(A_BOLD);

    // colored title
    if (has_colors()) {

        attrset(COLOR_PAIR(CYAN));
        char *title = "gdb-tuiffic";
        center_str(0, title);
        attrset(COLOR_PAIR(WHITE));

    // uncolored title
    } else {
        char *title = "gdb-tuiffic";
        printw("%s\n", title);
        refresh();
    }

    attroff(A_BOLD);
}

/*
   render string in center of window
*/
static void center_str (int row, char *str)
{
    int len;
    int indent;
    int y;
    int width;

    len = strlen(str);
    getmaxyx(stdscr, y, width);
    indent = (width - len) / 2;
    mvaddstr(row, indent, str);
    printw("\n");
    refresh();
}


/*
    create new window
*/
WINDOW *create_window(WINDOW *window_name)
{
    if (window == NULL) {
        // TODO: print window name
        fprintf(stderr, "ERROR: Unable to create window \"%s\"\n", "window");
        exit(EXIT_FAILURE);
    } else {
        WINDOW *window_name
    }
}
