
/*
   gdb-tuiffic
 */

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ncursesw/ncurses.h>

#include "create_colors.h"
#include "parse_config.h"
#include "render_screen.h"
#include "utilities.h"

#define VERSION         "0.0.1"
#define CMD_LEN         50
#define RUNNING         true

enum config_types {
    LAYOUTS
};

static void *allocate_config (int);


int main (void) 
{
    setlocale(LC_ALL, "");  // needed for wide border characters

    // initialize Ncurses, create color pairs
    initscr ();
    create_colors ();

    // create data structures to hold parsed CONFIG_FILE data (parse_config.h)
    layouts_t *layouts = allocate_config (LAYOUTS);

    // parse configuration file
    parse_config (layouts);

    // render screen
    int li = 0;                 // index of first layout
    render_screen (li, layouts);

    //
    //  Main loop
    //  ---------
    //  q   - quit
    //
    int ch;
    cbreak ();      // disable need to press Enter
    noecho ();      // do not display pressed character

        // read key presses
    while ((ch = getch()) != ERR) {

        // quit
        if (ch == 'q')
            break;

        // render screen
        render_screen (li, layouts);

        //nodelay(stdscr, true);
        //nodelay(stdscr, false);
    }

    endwin ();
    return 0;
}


/*
    Allocate memory for configuration structs
*/
static void *allocate_config (int config)
{
    void *config_ptr = NULL;

    switch (config) {

        // layouts_t
        case LAYOUTS:
            config_ptr = (layouts_t*) malloc (sizeof (layouts_t));
            ((layouts_t *) config_ptr)->num = 0;
            break;

        default:
            endwin ();
            pfem  ("Configuration struct enum not recognized\n");
            exit (EXIT_FAILURE);
    }

    if (config_ptr == NULL) {
        endwin ();
        pfem ("layouts_t allocation failed\n");
        exit (EXIT_FAILURE);
    } else {
        return config_ptr;
    }
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

