
/*
   Render Ncurses screen
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#include <ctype.h>
#include <ncurses.h>

#include "render_screen.h"
#include "parse_config.h"
#include "create_colors.h"
#include "utilities.h"

#define TITLE_OFFSET 1          // temporary, will be set by config file


static void create_layout (int win_rows, int win_cols, int li, layouts_t *layouts);
static windows_t *allocate_window (void);
static WINDOW *render_window (char *window_name, int rows, int cols, int pos_y, int pos_x);
void fix_corners (windows_t *win);
static WINDOW* create_new_window (int rows, int cols, int y, int x);
static WINDOW* create_sub_window (WINDOW *parent, int rows, int cols, int y, int x);


/*
    Render screen
*/
void render_screen ( int i, 
                     layouts_t *layouts)
{

    int scr_rows = getmaxy (stdscr);
    int scr_cols  = getmaxx (stdscr);
    int header_rows = 1;

    // calculate all layouts if program just started or if
    // the screen size changed
    if (scr_rows != layouts->scr_height || 
        scr_cols != layouts->scr_width) {
        for (int i = 0; i < layouts->num; i++) {
            create_layout (scr_rows - header_rows, scr_cols, i, layouts);
        }
    }

    // render title
    WINDOW *header = create_new_window (header_rows, COLS, 0, 0);
    char *title = "termIDE";
    wattron (stdscr, A_BOLD | COLOR_PAIR(GREEN_BLACK));
    mvwprintw (header, 0, 3, "%s", title);
    wattroff (stdscr, A_BOLD | COLOR_PAIR(GREEN_BLACK));
    refresh ();
    wrefresh (header);

    // render current layout's windows
    int pi = 0;
    windows_t *curr_window = layouts->windows [i];

        // loop through windows
    do {
        char *win_title = "test";

        // render, store window
        curr_window->win = render_window (
                        win_title, 
                        curr_window->rows,
                        curr_window->cols,
                        curr_window->y + header_rows,
                        curr_window->x);

        // next window
        curr_window = curr_window->next;

    } while (curr_window != NULL);

    // fix window border corners
    fix_corners (layouts->windows [i]);
}



/*
    Create layout
    ------------
    Allocate memory for window_t structs  (parse_config.h)
    Calculate position, dimensions

    Parameters:

        win_rows == main window height
        win_cols == main window width
        li       == layouts_t struct index
        layouts  == layouts_t struct

*/
static void create_layout ( int win_rows,
                            int win_cols,
                            int li,
                            layouts_t *layouts)
{
    // create matrices for number of rows, columns per segment
    //
    //      * * *
    //      * * *
    //      * * *
    //      * * *
    //
    //      win_rows: 41  win_cols: 83
    //      row_ratio: 4  col_ratio: 3
    //      floor_segm_rows: 10  floor_segm_cols: 27
    //      segm_row_rem: 1  segm_col_rem: 2
    //
    //      segm_rows:
    //      --------
    //      11  11  11
    //      10  10  10
    //      10  10  10
    //      10  10  10
    //
    //      segm_cols:
    //      --------
    //      28  28  27
    //      28  28  27
    //      28  28  27
    //      28  28  27
    //
    int y, x;
    int row_ratio = layouts->row_ratios [li];
    int col_ratio = layouts->col_ratios [li];
    int floor_segm_rows = win_rows / row_ratio;  // floor row amount per segment
    int floor_segm_cols = win_cols / col_ratio;
    int segm_rows [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};
    int segm_cols [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};

        // calculate total row, column remainders
    int segm_row_rem = win_rows - (floor_segm_rows * row_ratio);
    int segm_col_rem = win_cols - (floor_segm_cols * col_ratio);

    /*
    // PRINT
    mvprintw (5, 4, "row_ratio: %d  col_ratio: %d",
            row_ratio, col_ratio);
    mvprintw (4, 4, "win_rows: %d  win_cols: %d",
            win_rows, win_cols);
    mvprintw (6, 4, "floor_segm_rows: %d  floor_segm_cols: %d", 
            floor_segm_rows, floor_segm_cols);
    mvprintw (7, 4, "segm_row_rem: %d  segm_col_rem: %d", 
            segm_row_rem, segm_col_rem);
    */

        // add base segment rows, columns
        // distribute remainders
    for (y = 0; y < row_ratio; y++) {

        int col_rem = segm_col_rem;

        for (x = 0; x < col_ratio; x++) {

            // base
            segm_rows [y][x] = floor_segm_rows + ((segm_row_rem > 0) ? 1 : 0);
            segm_cols [y][x] = floor_segm_cols + ((col_rem > 0) ? 1 : 0);

            col_rem -= 1;
        }
        segm_row_rem -= 1;
    }

    /*
    // PRINT
    char row_label [] = "segm_rows:";
    char col_label [] = "segm_cols:";
    print_int_matrix (row_label, 9, segm_rows, row_ratio, col_ratio);
    print_int_matrix (col_label, 16, segm_cols, row_ratio, col_ratio);
    */

    // create matrices for top left y and x segment coordinates
    //
    //     x - -
    //     - - -        
    //     - - -        
    //
    int segm_ys [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};
    int segm_xs [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};

    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {

            // y
            int yc = 0;
            while (yc < y) {
                segm_ys [y][x] += segm_rows [yc][x];
                yc += 1;
            }

            // x
            int xc = 0;
            while (xc < x) {
                segm_xs [y][x] += segm_cols [y][xc];
                xc += 1;
            }
        }
    }

    /*
    // PRINT
    char y_label [] = "segm_ys:";
    char x_label [] = "segm_xs:";
    print_int_matrix (y_label, 23, segm_ys, row_ratio, col_ratio);
    print_int_matrix (x_label, 30, segm_xs, row_ratio, col_ratio);
    */

    // create (un)used segments matrix
    //
    //         0000
    //         0000
    //         0000
    //
    int used_segm_matrix [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};

    // create windows_t pointers for arranging
    // layouts->windows linked list
    windows_t *curr_window = NULL;
    windows_t *prev_window = NULL;

    // get layout matrix
    char **layout_matrix = (char **) layouts->matrices [li];

    /*
    // PRINT
    int row = 37;
    for (int i = 0; i < row_ratio; i++) {
        int col = 4;
        for (int j = 0; j < col_ratio; j++) {
            mvprintw (row, col, "%c", layout_matrix [i][j]);
            col += 4;
        }
        row += 1;
    }
    */

    // create windows
    char ch;
    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {

            // if unused segment
            if (used_segm_matrix [y][x] == 0) {

                // create window
                curr_window = allocate_window ();
                curr_window->next = NULL;

                // set head window or link previous
                if (y == 0 && x == 0) {
                    layouts->windows [li] = curr_window; // head
                } else {
                    prev_window->next = curr_window;    // link previous
                }

                // add key from matrix
                ch = layout_matrix [y][x];
                curr_window->key = ch;

                // calculate rows, cols
                //  
                //         x──a
                //         │ssa
                //         │ssc
                //         bbww
                //
                    // rows
                int yi = y;
                curr_window->rows = 0;

                while (yi < row_ratio) {
                    if (layout_matrix [yi][x] == ch) {
                        curr_window->rows += segm_rows [yi][x];
                        yi += 1;
                    } else {
                        break;
                    }
                }

                    // cols
                int xi = x;
                curr_window->cols = 0;
                while (xi < col_ratio) {
                    if (layout_matrix [y][xi] == ch) {
                        curr_window->cols += segm_cols [y][xi];
                        xi += 1;
                    } else {
                        break;
                    }
                }

                // set coordinates
                curr_window->y = segm_ys [y][x];
                curr_window->x = segm_xs [y][x];

                // create overlapping borders
                if (y > 0) {
                    curr_window->y -= 1;
                    curr_window->rows += 1;
                }
                if (x > 0) {
                    curr_window->x -= 1;
                    curr_window->cols += 1;
                }

                // set used segments
                //
                //      1110
                //      1110
                //      1110
                //      0000
                //
                for (int i = y; i < yi; i++) {
                    for (int j = x; j < xi; j++) {
                        used_segm_matrix [i][j] = 1;
                    }
                }

                // set previous window for linking
                prev_window = curr_window;
            }
        }
    }

    curr_window = NULL;
    prev_window = NULL;
}


/*
    Allocate memory for new windows_t struct
*/
static windows_t *allocate_window (void)
{
    windows_t *window = (windows_t *) malloc (sizeof (windows_t));
    if (window) {
        return window;
    } else {
        endwin ();
        pfem ("Unable to allocate memory for windows_t struct");
        exit (EXIT_FAILURE);
    }
}


/*
    Create sub window
    -------------------
    rows, cols      - height, width
    pos_y, pos_x    - window position in terms of top left corner
*/
static WINDOW *render_window (
        char *window_name, 
        int rows, 
        int cols, 
        int y, 
        int x) 
{
    //int screen_width     = getmaxx (stdscr);
    //int screen_height    = getmaxy (stdscr);    

    // create window object
    WINDOW *win = create_new_window (rows, cols, y, x);

    // calculate title indent
    int title_length = strlen (window_name);
    int title_indent = (cols - title_length) / 2;

    // render border
    wattron (win, COLOR_PAIR(BORDER_COLOR));
    wborder (win, 0,0,0,0,0,0,0,0);
    wattroff (win, COLOR_PAIR(BORDER_COLOR));

    /*
    // render title
    char *space = " ";
    wattron (win, A_BOLD | COLOR_PAIR(TITLE_COLOR));
    mvwaddstr (win, 0, title_indent - 1, space);
    mvwaddstr (win, 0, title_indent, window_name);
    mvwaddstr (win, 0, title_indent + title_length, space);
    wattroff (win, A_BOLD | COLOR_PAIR(TITLE_COLOR));
    */

    refresh ();
    wrefresh (win);

    return win;
}



/*
    Fix single ncurses border corner character
    ---------------
    Called by fix_corners()
    Returns correct corner character int

        y, x    - screen coordinates

*/
static int fix_corner_char (
            int y, 
            int x)
{
    int i;
    int ch;

    // get border character
    ch = mvwinch (curscr, y, x) & A_CHARTEXT;

    // if already corrected, return current character
    int corrected_chars [] = {
        ACS_LTEE,
        ACS_RTEE,
        ACS_BTEE,
        ACS_TTEE,
        ACS_PLUS
    };
    for (i = 0; i < 5; i++) {
        if (ch == (corrected_chars [i] & A_CHARTEXT))
            return corrected_chars [i];
    }

    // check which borders are present
    //
    //      t         │       {t, r, b, l}
    //    l c r     ─ c  -->  {1, 0, 0, 1}
    //      b                             
    //
    int borders [4] = {0};
    int horiz_line = ACS_HLINE & A_CHARTEXT;
    int vert_line = ACS_VLINE & A_CHARTEXT;

        // top
    if (y > TITLE_OFFSET) {
        ch = mvwinch (curscr, y - 1, x) & A_CHARTEXT;
        if (ch == vert_line)
            borders [0] = 1;
    }

        // right
    if (x < (COLS - 1)) {
        ch = mvwinch (curscr, y, x + 1) & A_CHARTEXT;
        if (ch == horiz_line)
            borders [1] = 1;
    }

        // bottom
    if (y < (LINES - 1)) {
        ch = mvwinch (curscr, y + 1, x) & A_CHARTEXT;
        if (ch == vert_line)
            borders [2] = 1;
    }

        // left
    if (x > 0) {
        ch = mvwinch (curscr, y, x - 1) & A_CHARTEXT;
        if (ch == horiz_line)
            borders [3] = 1;
    }

    // return corrected corner character

        // {1,1,1,0} --> ├  
    if (borders[0] && borders[1] && borders[2] && !borders[3])
        return ACS_LTEE;

        // {1,0,1,1} --> ┤
    else if (borders[0] && !borders[1] && borders[2] && borders[3])
        return ACS_RTEE;

        // {0,1,1,1} --> ┬
    else if (!borders[0] && borders[1] && borders[2] && borders[3])
        return ACS_TTEE;

        // {1,1,0,1} --> ┴
    else if (borders[0] && borders[1] && !borders[2] && borders[3])
        return ACS_BTEE;

        // {1,1,1,1} --> ┼
    else if (borders[0] && borders[1] && borders[2] && borders[3])
        return ACS_PLUS;

    else 
        return 0;
}


/*
    Fix broken border corners caused by overlapping in create_layout()
   
          │       │
         ─┐ -->  ─┤
          │       │
   
    Passed first window in layouts_t's windows_t linked list
*/
void fix_corners (windows_t *win)
{
    int of = TITLE_OFFSET;

    do {
        int y = win->y;
        int x = win->x;
        int rows = win->rows;
        int cols = win->cols;
        //
        int tl = 0;
        int tr = 0;
        int bl = 0;
        int br = 0;

        // top left
        tl = fix_corner_char (y + of, x);

        // top right
        tr = fix_corner_char (y + of, x + (cols - 1));

        // bottom left
        bl = fix_corner_char (y + (rows - 1) + of, x);

        // bottom right
        br = fix_corner_char (y + (rows - 1) + of, x + (cols - 1));

        // set corners
        //
        //      wborder (win, ls, rs, ts, bs, tl, tr, bl, br)
        //
        WINDOW *w = win->win;
            //
        wattron (w, COLOR_PAIR(BORDER_COLOR));
        wborder (w, 0, 0, 0, 0, tl, tr, bl, br);
        wattroff (w, COLOR_PAIR(BORDER_COLOR));
        refresh ();
        wrefresh (w);

        // next window
        win = win->next;

    } while (win != NULL);
}


/*
    Check window
*/
static void check_window (WINDOW *win)
{
    if (win == NULL) {
        endwin();
        pfem  ("Unable to create window\n");
        exit (EXIT_FAILURE);
    }
}


/*
    Create new window
*/
static WINDOW* create_new_window (
        int rows,
        int cols,
        int y,
        int x)
{
    WINDOW *win = newwin (rows, cols, y, x);
    check_window (win);
    return win;
}


/*
    Create subwindow relative to parent
*/
static WINDOW* create_sub_window (
        WINDOW *parent,
        int rows,
        int cols,
        int y,
        int x)
{
    WINDOW *swin = derwin (parent, rows, cols, y, x);
    check_window (swin);
    return swin;
}
