
/*
   Render Ncurses screen
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>
#include <ncursesw/ncurses.h>

#include "render_screen.h"
#include "parse_config.h"
#include "create_colors.h"
#include "utilities.h"


#define TITLE_OFFSET  1

static void create_layout (int win_rows, int win_cols, int li, layouts_t *layouts);
static windows_t *allocate_window (void);
static WINDOW *render_sub_window (WINDOW *window, char *window_name, int rows, int cols, int pos_y, int pos_x);
static void fix_border_char (int y, int x);
static void fix_corners (int li, layouts_t *layouts);


/*
    Render screen
*/
// #render
void render_screen ( int i, 
                     layouts_t *layouts)
{
    // check if screen width, height changed
    int scr_rows = getmaxy (stdscr);
    int scr_cols  = getmaxx (stdscr);

    // calculate all layouts if program just started or if
    // the screen size changed
    int main_rows = scr_rows - TITLE_OFFSET;
    int main_cols = scr_cols;
    if (scr_rows != layouts->scr_height || 
        scr_cols != layouts->scr_width) {
        for (int i = 0; i < layouts->num; i++) {
            create_layout (main_rows, main_cols, i, layouts);
        }
    }

    // render title
    char *title = "termIDE";
    wattron (stdscr, A_BOLD | COLOR_PAIR(GREEN_BLACK));
    mvprintw (0, 3, "%s", title);
    wattroff (stdscr, A_BOLD | COLOR_PAIR(GREEN_BLACK));
    refresh ();

    // render windows
        // main window area
    WINDOW *main;
    main = newwin (main_rows, main_cols, 0 + TITLE_OFFSET, 0);
    
    /*
    // PRINT
    print_layouts (1, layouts);             // first layout
    print_layouts (layouts->num, layouts);  // all layouts
    */

    windows_t *curr_window = layouts->windows [i];
    //int act_i = 0;
    //char code [3];
    //char title [30];
    char *win_title = "test";
    do {

        // get title, action code
        //strcpy (title, window_names [act_i]);
        //strcpy (code,  action_codes [act_i]);

        // print window data
        render_sub_window (
                        main, 
                        win_title, 
                        curr_window->rows,
                        curr_window->cols,
                        curr_window->y,
                        curr_window->x);

        // next window
        curr_window = curr_window->next;

    } while (curr_window != NULL);

    fix_corners (i, layouts);
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
// #layout
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


// allocate memory for new windows_t struct
// #allocate
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
// #subwin
static WINDOW *render_sub_window(
        WINDOW *main, 
        char *window_name, 
        int rows, 
        int cols, 
        int y, 
        int x) 
{
    //int screen_width     = getmaxx (stdscr);
    //int screen_height    = getmaxy (stdscr);    

    // create window object
    WINDOW *sub_window;
    sub_window = derwin (main, rows, cols, y, x);
    if (sub_window == NULL) {
        endwin();
        pfem  ("Unable to create window\n");
        pfemo ("%s %d %d %d %d\n", window_name, rows, cols, y, x);
        exit (EXIT_FAILURE);
    }

    // calculate title indent
    int title_length = strlen (window_name);
    int title_indent = (cols - title_length) / 2;

    // render border
    wattron (sub_window, COLOR_PAIR(BORDER_COLOR));
    wborder (sub_window, 0,0,0,0,0,0,0,0);
    wattroff (sub_window, COLOR_PAIR(BORDER_COLOR));

    // render title
    char *space = " ";
    wattron (sub_window, A_BOLD | COLOR_PAIR(TITLE_COLOR));
    mvwaddstr (sub_window, 0, title_indent - 1, space);
    mvwaddstr (sub_window, 0, title_indent, window_name);
    mvwaddstr (sub_window, 0, title_indent + title_length, space);
    wattroff (sub_window, A_BOLD | COLOR_PAIR(TITLE_COLOR));

    refresh ();
    wrefresh (sub_window);

    return sub_window;
}

/*
    Fix current ncurses corner border character
    ---------
    Used by fix_corners() below
*/
static void fix_border_char (int y, int x)
{
    // get screen dimensions
    int scr_rows = getmaxy (stdscr);
    int scr_cols  = getmaxx (stdscr);

    // check which borders are present
    //
    //      t         │       {t, r, b, l}
    //    l c r     ─ c  -->  {1, 0, 0, 1}
    //      b                             
    //
    setlocale(LC_ALL, "");
    cchar_t cch;
    int borders [4] = {0};

    // PRINT
    static int row = 3;
    int col = 0;
    mvin_wch (y, x, &cch);
    mvadd_wch (row, col, &cch);
    row++;
    refresh();

    noecho ();
    int ch = getch();
    /*
    col += 4;
    mvprintw (row, col, "%d", y);
    col += 4;
    mvprintw (row++, col, "%d", x);
    */

    /*
    // create side border characters
    wchar_t wb_vert = L'│';
    wchar_t wb_horiz = L'─';
    cchar_t b_vert, b_horiz;
    setcchar (&b_vert, &wb_vert, A_NORMAL, 0, NULL);
    setcchar (&b_horiz, &wb_horiz, A_NORMAL, 0, NULL);
    */


    /*
        // top
    if (y > TITLE_OFFSET) {
        mvin_wch (y+1, x, &cch);
        mvadd_wch (row, col, &cch);
        col += 3;
        if (memcmp (&cch, &b_vert, sizeof (cchar_t)) == 0) {
            borders [0] = 1;
        }
    }

        // right
    if (x < (scr_cols - 1)) {
        mvin_wch (y, x+1, &cch);
        mvaddwstr (row, col, &cch);
        col += 3;
        if (memcmp (&cch, &b_horiz, sizeof (cchar_t)) == 0) {
            borders [1] = 1;
        }
    }

        // bottom
    if (y < (scr_rows - 1)) {
        mvin_wch (y-1, x, &cch);
        mvaddwstr (row, col, &cch);
        col += 3;
        if (memcmp (&cch, &b_vert, sizeof (cchar_t)) == 0) {
            borders [2] = 1;
        }
    }

        // left
    if (x > 0) {
        mvin_wch (y, x-1, &cch);
        mvaddwstr (row, col, &cch);
        col += 3;
        if (memcmp (&cch, &b_horiz, sizeof (cchar_t)) == 0) {
            borders [3] = 1;
        }
    }


    // PRINT
    mvprintw (row++, col, "y:%d x:%d  %d %d %d %d", 
            y, x, borders[0], borders[1], borders[2], borders[3]);
    */

    /*
    // fix character
    //
    //   │       │
    //  ─┐ -->  ─┤ 
    //   │       │
    //
    
        // {1,1,1,0} --> ├  
    if (borders[0] && borders[1] && borders[2] && !borders[3]) {
        bch [0] = L'├';
        bch [1] = L'\0';
        mvaddwstr (y, x, bch);
    }

        // {1,0,1,1} --> ┤
    else if (borders[0] && !borders[1] && borders[2] && borders[3]) {
        bch [0] = L'┤';
        bch [1] = L'\0';
        mvaddwstr (y, x, bch);
    }

        // {0,1,1,1} --> ┬
    else if (!borders[0] && borders[1] && borders[2] && borders[3]) {
        bch [0] = L'┬';
        bch [1] = L'\0';
        mvaddwstr (y, x, bch);
    }

        // {1,1,0,1} --> ┴
    else if (borders[0] && borders[1] && !borders[2] && borders[3]) {
        bch [0] = L'┴';
        bch [1] = L'\0';
        mvaddwstr (y, x, bch);
    }

        // {1,1,1,1} --> ┼
    else if (borders[0] && borders[1] && borders[2] && borders[3]) {
        bch [0] = L'┼';
        bch [1] = L'\0';
        mvaddwstr (y, x, bch);
    }
    */
}


/*
    Fix all of current layout's broken border corners 
    caused by overlapping in create_layout()

          │       │
         ─┐ -->  ─┤
          │       │
*/
static void fix_corners ( int li,
                          layouts_t *layouts)
{
    int of = TITLE_OFFSET;
    windows_t *win = layouts->windows [li];

    do {
        // top left
        fix_border_char (win->y + of, win->x);

        // top right
        fix_border_char (win->y + of, win->x + (win->cols - 1));

        // bottom left
        fix_border_char (win->y + (win->rows - 1) + of, win->x);

        // bottom right
        fix_border_char (win->y + (win->rows - 1) + of, win->x + (win->cols - 1));

        // next window
        win = win->next;

    } while (win != NULL);
}
