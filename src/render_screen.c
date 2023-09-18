
/*
   Render Ncurses screen
*/


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#include "render_screen.h"
#include "apply_config.h"
#include "colors.h"
#include "utilities.h"

static void create_sub_window (WINDOW *window, 
                               char *window_name, 
                               int   rows, 
                               int   cols, 
                               int   pos_y, 
                               int   pos_x);


/*
    Render screen
*/
void 
render_screen (struct layouts *layouts)
{

    // render outer border, title
    int screen_width  = getmaxx (stdscr);
    int screen_height = getmaxy (stdscr);
    create_sub_window(stdscr, "", screen_height, screen_width, 0, 0);
    create_sub_window(stdscr, "gdb-tuiffic", 3, screen_width, 0, 0);


    int y = 3;
    int x = 4;
    move (y++, x);
    printw ("Number of layouts: %d", layouts->num_layouts);

    for (int i = 0; i < layouts->num_layouts; i++) {

        mvprintw (y++, x, "Layout: %d, windows: %d, ", 
                  layouts->labels [i],
                  layouts->num_windows [i]);

        // layout windows
        mvprintw (y++, x, "Window:");
        while (true) {
            mvprintw (y++, x, "\"%c\", %s, rows: %d, cols: %d, y: %d, x: %d",
                                layouts->windows[i]->symbol,
                                layouts->windows[i]->title,
                                layouts->windows[i]->rows,
                                layouts->windows[i]->cols,
                                layouts->windows[i]->y,
                                layouts->windows[i]->x);

            if (layouts->windows[i]->next == NULL) break;
        }
        y += 1;
    }
}


/*
    Create sub window
    -------------------
    rows, cols      - height, width
    pos_y, pos_x    - window position in terms of top left corner
*/
static void
create_sub_window(
        WINDOW *window, 
        char *window_name, 
        int rows, 
        int cols, 
        int pos_y, 
        int pos_x) 
{
    //int screen_width     = getmaxx (stdscr);
    //int screen_height    = getmaxy (stdscr);    

    // create window object
    window = subwin (window, rows, cols, pos_y, pos_x);
    if (window == NULL) {
        fprintf (stderr, "Unable to create window\n");
        exit (EXIT_FAILURE);
    }

    // calculate title indent
    int title_length = strlen (window_name);
    int title_indent = (cols - title_length) / 2;

    // render title
    wattron (window, A_BOLD | COLOR_PAIR(TITLE_COLOR) | A_UNDERLINE);
    mvwaddstr (window, 1, title_indent, window_name);
    wattroff (window, A_BOLD | COLOR_PAIR(TITLE_COLOR) | A_UNDERLINE);

    // render border
    wattron (window, COLOR_PAIR(BORDER_COLOR));
    wborder (window, 0,0,0,0,0,0,0,0);
    wattroff (window, COLOR_PAIR(BORDER_COLOR));

    refresh ();
    wrefresh (window);
}


