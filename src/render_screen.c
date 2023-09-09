
/*
    Ncurses rendering utilities
    --------------
    Public:
        render_window
    Static:
        create_colors
        render_title
        center_string
*/

// TODO: implement normal, insert (cmd) modes
// TODO: external config file
    // "visual" layout
    // component colors

#include <ncurses.h>
#include <stdbool.h>

#include "render_screen.h"
#include "utilities.h"

#define BORDER_COLOR        BLUE_BLACK
#define TITLE_COLOR         CYAN_BLACK
#define MAX_WIN_CONFIG_Y    10
#define MAX_WIN_CONFIG_X    10

static char layout[MAX_WIN_CONFIG_Y][MAX_WIN_CONFIG_X] = {0};


/*
    Prototypes
*/
static void create_colors();
static WINDOW* create_sub_window(WINDOW *window, char *window_name, int rows, int cols, int pos_y, int pos_x);
void apply_config_file();


/*
    Render screen
*/
void 
render_screen()
{
    // create color pairs
    create_colors();
    apply_config_file();

    int screen_width     = getmaxx(stdscr);
    int screen_height    = getmaxy(stdscr);    

    /*
        Create windows
    */

    // terminal screen
    WINDOW *screen = create_sub_window(stdscr, "gdb-tuiffic", screen_height, screen_width, 0, 0);
}


/*
   Create font_background color pairs
   ---------------------
   - check current terminal's color capability with:  
        $ make colors
*/
static void 
create_colors()
{
    if (has_colors()) {

        start_color();

        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
        init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);
        init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
        init_pair(WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);
    }
}


/*
    Create sub window
    -------------------
    rows, cols      - height, width
    pos_y, pos_x    - window position in terms of top left corner
*/
static WINDOW*
create_sub_window(
        WINDOW *window, 
        char *window_name, 
        int rows, 
        int cols, 
        int pos_y, 
        int pos_x) 
{
    // create main window
    window = subwin(window, rows, cols, pos_y, pos_x);
    if (window == NULL) {
        fprintf(stderr, "Unable to create window\n");
        exit(EXIT_FAILURE);
    }

    // calculate title indent
    int title_length = strlen(STR(window));
    int title_indent = (cols - title_length) / 2;

    // render title
    wattron(window, A_BOLD | COLOR_PAIR(TITLE_COLOR) | A_UNDERLINE);
    mvwaddstr(window, 1, title_indent, window_name);
    wattroff(window, A_BOLD | COLOR_PAIR(TITLE_COLOR) | A_UNDERLINE);

    // render border
    wattron(window, COLOR_PAIR(BORDER_COLOR));
    wborder(window, 0,0,0,0,0,0,0,0);
    wattroff(window, COLOR_PAIR(BORDER_COLOR));

    refresh();
    wrefresh(window);
}


