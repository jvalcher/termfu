
/*
   Render Ncurses screen
   ---------------

    Contents:

        Tag         Description
        ---         -----------
        #render     render_screen(), main render function
        #layout     create_layout(), create layout based on current screen size
        #allocate   allocate_window(), allocate memory for windows_t struct
        #subwin     create_sub_window(), render window

*/


#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ncurses.h>

#include "render_screen.h"
#include "parse_config.h"
#include "create_colors.h"
#include "utilities.h"

static void create_layout (int win_rows, int win_cols, int li, layouts_t *layouts);
static windows_t *allocate_window (void);
static WINDOW *render_sub_window (WINDOW *window, char *window_name, int rows, int cols, int pos_y, int pos_x);

/*
    Render screen
*/
// #render
void render_screen (
        int i, 
        layouts_t *layouts)
{
    // check if screen width, height changed
    int scr_rows = getmaxy (stdscr);
    int scr_cols  = getmaxx (stdscr);

    // calculate all layouts if program just started or if
    // the screen size changed
    int title_offset = 2;
    int main_rows = scr_rows - title_offset;
    int main_cols = scr_cols;
    if (scr_rows != layouts->scr_height || 
        scr_cols != layouts->scr_width) {
        for (int i = 0; i < layouts->num; i++) {
            create_layout (main_rows, main_cols, i, layouts);
        }
    }

    // render windows
        // title
    render_sub_window (stdscr, "termIDE", 3, scr_cols, 0, 0);

        // main window area
    WINDOW *main;
    main = newwin (main_rows, main_cols, 0 + title_offset, 0);
    
    // print layout(s) data  (for development)
    //print_layouts (1, layouts);                 // one layout
    //print_layouts (layouts->num, layouts);    // all layouts

    /*
        // main sub windows from layouts->windows [i]
    windows_t *windows = layouts->windows [i];
    int act_i = 0;
    char code [3];
    char title [30];
    //int row = 4;
    do {
        memset (code,  '\0', (sizeof (char) * 3));
        memset (title, '\0', (sizeof (char) * 30));

        // find symbol index for actions, window names
        for (int i = 0; i < NUM_ACTIONS; i++) {
            if (windows->symb == action_keys [i]) {
                act_i = i;
                break;
            }
        }

        // get title, action code
        strcpy (title, window_names [act_i]);
        //strcpy (code,  action_codes [act_i]);

        //print_layouts (1, layouts);

        // print window data
        render_sub_window (
                        main, 
                        title, 
                        windows->rows,
                        windows->cols,
                        windows->y,
                        windows->x);

        // next window
        windows = windows->next;

    } while (windows != NULL);
    */
}


/*
    Create layout
    ------------
    Allocate memory for window_t structs  (parse_config.h)
    Calculate position, dimensions

    Parameters:

        win_rows == window height
        win_cols == window width
        li       == layouts_t struct index
        layouts  == layouts_t struct

*/
// #layout
static void create_layout (
        int win_rows,
        int win_cols,
        int li,
        layouts_t *layouts)
{

    // create matrices for segment rows, columns, coordinates
    //
    //      21 21 21
    //      21 21 21
    //      20 20 20    --> rows per segment
    //
    int i,j,k,pi;
    int y_ratio = layouts->y_ratios [li];
    int x_ratio = layouts->x_ratios [li];

        // rows (y), columns (x)
    int y_segm_len = win_rows / y_ratio;  // floor row amount per segment
    int x_segm_len = win_cols / x_ratio;
    int y_segm_lens [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};
    int x_segm_lens [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};

        // remainders
    int y_segm_rem = win_rows - (y_segm_len * y_ratio);
    int x_segm_rem = win_cols - (x_segm_len * x_ratio);

        // y,x coordinates (top left)
    int segm_ys [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};
    int segm_xs [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};

        // create matrices
    for (i = 0; i < y_ratio; i++) {
        for (j = 0; j < x_ratio; j++) {

            // rows, columns
            y_segm_lens [i][j] = y_segm_len + ((y_segm_rem-- > 0) ? 1 : 0);
            x_segm_lens [i][j] = x_segm_len + ((y_segm_rem-- > 0) ? 1 : 0);

            // y, x (TODO)
            pi = 0;
            while (pi++ < i) segm_ys [i][j] += y_segm_lens [pi][j] - 1;
            // ...

        }
    }

    /*
    // create segment remainder matrices to keep track of
    // which segments must add a single column or row
    // 
    //
    //      1110
    //      1110
    //      0000
    //
        // y remainders
    int y_segm_rem_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};
    int y_segm_len = win_rows / y_ratio;                  // floor row amount per segment
    int y_segm_rem = win_rows - (y_segm_len * y_ratio);   // remainder
    for (i = 0; i < y_ratio; i++) {
        for (j = 0; j < x_ratio; j++) {
            y_segm_rem_matrix [i][j] += (y_segm_rem > 0) ? 1 : 0;
        }
        y_segm_rem--;
    }


        // x remainders
    int x_segm_rem_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};
    int x_segm_len = win_cols / x_ratio;
    int x_segm_rem = win_cols - (x_segm_len * x_ratio);
    int x_rem;
    for (i = 0; i < y_ratio; i++) {
        x_rem = x_segm_rem;
        for (j = 0; j < x_ratio; j++) {
            x_segm_rem_matrix [i][j] += (x_rem-- > 0) ? 1 : 0;
        }
    }
    */

    //print_int_matrix (x_segm_rem_matrix, y_ratio, x_ratio);

    // create (un)used segments matrix
    //
    //         1110
    //         1110
    //         0000
    //
        // allocate
    int used_segm_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};

        // initialize to zero
    for (i = 0; i < y_ratio; i++) {
        for (j = 0; j < x_ratio; j++) {
            used_segm_matrix [i][j] = 0;
        }
    }

    // create windows_t pointers for arranging layout's 
    // windows linked list
    windows_t *curr_window = NULL;
    windows_t *prev_window = NULL;

    // get layout matrix  (parse_config.h)
    char **layout_matrix = (char **) layouts->matrices [li];

    // store top left coordinates of next segment
    int next_y;
    int next_x = 0;

    // loop through layout_matrix segments
    char ch;
    for (int y = 0; y < y_ratio; y++) {
        for (int x = 0; x < x_ratio; x++) {

            // if unused segment
            if (used_segm_matrix [y][x] == 0) {

                // get window symbol character
                ch = layout_matrix [y][x];

                // create window
                curr_window = allocate_window ();
                curr_window->rows = 0;
                curr_window->cols = 0;
                curr_window->y = 0;
                curr_window->x = 0;
                curr_window->next = NULL;

                // set head or link previous window
                if (y == 0 && x == 0) {
                    layouts->windows [li] = curr_window;     // set head window
                } else {
                    prev_window->next = curr_window;    // else, link to previous window
                }

                // add symbol
                curr_window->symb = ch;

                // calculate y,x coordinates*, height, width
                //  
                //         *──aa
                //         │ssaa
                //         │sscc
                //         bbwwr
                //
                    // y
                int yi;
                curr_window->y = 0;
                for (yi = 0; yi < y; yi++) {
                    curr_window->y += y_segm_len;
                    curr_window->y += y_segm_rem_matrix [yi][x];
                }

                    // height in rows
                for (yi = y; yi < y_ratio; yi++) {
                    if (ch == layout_matrix [yi][x]) {


                        // rows
                        if (yi == (y_ratio - 1)) {      // bottom segment
                            curr_window->rows = win_rows - next_y;
                            break;
                        } else {
                            curr_window->rows += y_segm_len + y_segm_rem_matrix [yi][x];
                        }

                        // next y
                        next_y += curr_window->rows - 1;

                    } else {
                        break;
                    }
                }

                    // x, width in columns
                int xi;
                for (xi = x; xi < x_ratio; xi++) {

                    if (ch == layout_matrix [y][xi]) {

                        // x
                        if (xi == 0) {
                            curr_window->x = 0;
                            next_x = 0;
                        } else {
                            curr_window->x = next_x;
                        }

                        // rows
                        if (xi == x_ratio - 1) {    // most right segment
                            curr_window->cols = win_cols - next_x;
                            break;
                        } else {
                            curr_window->cols += x_segm_len + x_segm_rem_matrix [y][xi];
                        }

                        // next x
                        next_x += curr_window->cols - 1;

                    } else {
                        break;
                    }
                }

                // set used segments
                //
                //      1110
                //      1110
                //      0000
                //
                for (i = y; i < yi; i++) {
                    for (j = x; j < xi; j++) {
                        used_segm_matrix [i][j] = 1;
                    }
                }

                // set previous window
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

    // render title
    wattron (sub_window, A_BOLD | COLOR_PAIR(TITLE_COLOR) | A_UNDERLINE);
    mvwaddstr (sub_window, 1, title_indent, window_name);
    wattroff (sub_window, A_BOLD | COLOR_PAIR(TITLE_COLOR) | A_UNDERLINE);

    // render border
    wattron (sub_window, COLOR_PAIR(BORDER_COLOR));
    wborder (sub_window, 0,0,0,0,0,0,0,0);
    wattroff (sub_window, COLOR_PAIR(BORDER_COLOR));

    refresh ();
    wrefresh (sub_window);

    return sub_window;
}

