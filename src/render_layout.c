
/*
   Render Ncurses layout (not including window data)
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>
#include <stdbool.h>
#include <ncurses.h>

#include "data.h"
#include "render_layout.h"
#include "utilities.h"

char *prog_title = "termIDE";
int   scr_rows;
int   scr_cols;
int   header_offset;

static void    create_layout        (int, int, layout_t*);
static WINDOW *create_new_window    (int, int, int, int);
static void    render_main_title    (WINDOW*, char*, char*);
static void    render_borders       (window_t*);
static void    fix_corners          (window_t*);
static void    render_header_titles (layout_t*);
static void    render_window_titles (layout_t*);



/*
    Render layouts
*/
void render_layout (layout_t *layout)
{
    // Get screen dimensions
    scr_rows = getmaxy (stdscr);
    scr_cols = getmaxx (stdscr);

    // Set offset for number of header lines
    header_offset = layout->num_hdr_key_rows + 1;

    // Calculate current layout's window dimensions
    create_layout (scr_rows - header_offset, scr_cols, layout);

    // Print each layout's info and shortcuts for debugging  ($ make layouts)
    #ifdef LAYOUT
    print_layouts (PRINT_LAYOUTS, layout);
    #endif

    // Create header window
    layout->header = create_new_window (header_offset, COLS, 0, 0);

    // Render main header title (<program name> : <layout name>)
    render_main_title (layout->header, prog_title, layout->label);

    // Render window borders
    render_borders (layout->windows);

    // Fix window border corners to account for overlapping
    fix_corners (layout->windows);

    // Render header titles
    render_header_titles (layout);

    // Render window titles
    render_window_titles (layout);
}



/*
    Create layout
    ------------
    Calculate position, dimensions of plugin windows
    Allocate memory for window_t structs in layout->windows  (data.h)

    Parameters:

        win_rows   - main window height
        win_cols   - main window width
        li         - layout_t struct index
        layout    - layout_t struct
*/
static void create_layout (int win_rows,
                           int win_cols,
                           layout_t *layout)
{
    // create matrices for number of rows, columns per segment
    //
    //      Example: 
    //
    //          * * *
    //          * * *
    //          * * *
    //          * * *
    //
    //          win_rows: 41  win_cols: 83
    //          row_ratio: 4  col_ratio: 3
    //          floor_segm_rows: 10  floor_segm_cols: 27
    //          segm_row_rem: 1  segm_col_rem: 2
    //
    //          segm_rows:
    //          --------
    //          11  11  11
    //          10  10  10
    //          10  10  10
    //          10  10  10
    //
    //          segm_cols:
    //          --------
    //          28  28  27
    //          28  28  27
    //          28  28  27
    //          28  28  27
    //
    int y, x;
    int row_ratio = layout->row_ratio;
    int col_ratio = layout->col_ratio;
    int floor_segm_rows = win_rows / row_ratio;  // floor row amount per segment
    int floor_segm_cols = win_cols / col_ratio;
    int segm_rows [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};
    int segm_cols [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};

        // calculate total row, column remainders
    int segm_row_rem = win_rows - (floor_segm_rows * row_ratio);
    int segm_col_rem = win_cols - (floor_segm_cols * col_ratio);

        // add base segment rows, columns
        // distribute remainders
    for (y = 0; y < row_ratio; y++) {

        int col_rem = segm_col_rem;

        for (x = 0; x < col_ratio; x++) {

            // base
            segm_rows [y][x] = floor_segm_rows + (segm_row_rem > 0 ? 1 : 0);
            segm_cols [y][x] = floor_segm_cols + (col_rem > 0      ? 1 : 0);

            col_rem -= 1;
        }
        segm_row_rem -= 1;
    }

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

    // create (un)used segments matrix
    //
    //         0000
    //         0000
    //         0000
    //
    int used_segm_matrix [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0};

    // create window_t pointers for arranging
    // layout->windows linked list
    window_t *curr_window = NULL;
    window_t *prev_window = NULL;

    // get layout matrix
    char **layout_matrix = (char **) layout->win_matrix;

    // create windows
    char ch;
    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {

            // if unused segment
            if (used_segm_matrix [y][x] == 0) {

                // create window
                curr_window = (window_t *) malloc (sizeof (window_t));
                if (curr_window == NULL)
                    pfeme ("Unable to allocate memory for window_t struct");

                // set focus
                curr_window->selected = false;

                // set head window or link previous
                curr_window->win_next = NULL;
                if (y == 0 && x == 0) {
                    layout->windows = curr_window; // head
                } else {
                    prev_window->win_next = curr_window;    // link previous
                }

                // add key from matrix
                ch = layout_matrix [y][x];
                curr_window->key = ch;

                // calculate rows, cols
                //           
                //         ┌──a
                //         │ssa
                //         │ssc
                //         bbww
                //
                    // rows
                int yi = y;
                curr_window->win_rows = 0;
                while (yi < row_ratio) {
                    if (layout_matrix [yi][x] == ch) {
                        curr_window->win_rows += segm_rows [yi][x];
                        yi += 1;
                    } else {
                        break;
                    }
                }
                    // cols
                int xi = x;
                curr_window->win_cols = 0;
                while (xi < col_ratio) {
                    if (layout_matrix [y][xi] == ch) {
                        curr_window->win_cols += segm_cols [y][xi];
                        xi += 1;
                    } else {
                        break;
                    }
                }

                // set top left coordinates
                curr_window->win_y = segm_ys [y][x];
                curr_window->win_x = segm_xs [y][x];

                // overlap borders
                if (y > 0) {
                    curr_window->win_y -= 1;
                    curr_window->win_rows += 1;
                }
                if (x > 0) {
                    curr_window->win_x -= 1;
                    curr_window->win_cols += 1;
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
}



/*
    Create new window relative to stdscr
*/
static WINDOW* create_new_window (int rows,
                                  int cols,
                                  int y,
                                  int x)
{
    WINDOW *win = newwin (rows, cols, y, x);
    if (win == NULL)
        pfeme ("Unable to create window\n");

    return win;
}



/*
    Render main title 
    -----------
    termIDE : <current layout>
*/
static void render_main_title (WINDOW* header,
                               char *title,
                               char *layout)
{
    int title_len = strlen (title);
    char *colon = ":";

    // program name
    wattron (header, COLOR_PAIR(MAIN_TITLE_COLOR) | A_BOLD | A_UNDERLINE);
    mvwprintw (header, 1, 2, "%s", title);
    wattrset (header, A_NORMAL);

    // colon
    wattron (header, COLOR_PAIR(WHITE_BLACK));
    mvwprintw (header, 1, title_len + 3, "%s", colon);

    // current layout
    wattron (header, COLOR_PAIR(LAYOUT_TITLE_COLOR));
    mvwprintw (header, 1, title_len + 4, "%s", layout);
    attrset (A_NORMAL);

    refresh ();
    wrefresh (header);
}



/*
    Create sub window
    -------------------
    rows, cols      - height, width
    pos_y, pos_x    - window position in terms of top left corner
*/
static WINDOW *render_window (int rows, 
                              int cols, 
                              int y, 
                              int x) 
{
    // create window object
    WINDOW *win = create_new_window (rows, cols, y, x);
    if (win == NULL)
        pfeme  ("Unable to create window\n");

    // render border
    wattron (win, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
    wborder (win, 0,0,0,0,0,0,0,0);
    wattroff (win, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
    refresh ();
    wrefresh (win);

    return win;
}



/*
    Render window borders
*/
static void render_borders (window_t *curr_window)
{
    do {
        curr_window->win = render_window (
                        curr_window->win_rows,
                        curr_window->win_cols,
                        curr_window->win_y + header_offset,
                        curr_window->win_x);
        curr_window = curr_window->win_next;

    } while (curr_window != NULL);
}



/*
    Fix broken border corners caused by overlapping in create_layout()
   
          │       │
         ─┐ -->  ─┤
          │       │
   
    - Passed head of window_t linked list in layout_t
    - Uses fix_corner_char()
*/
    //
    static int fix_corner_char (int y, int x);
    //
static void fix_corners (window_t *win)
{
    int of = header_offset;

    do {
        int y     = win->win_y;
        int x     = win->win_x;
        int rows  = win->win_rows;
        int cols  = win->win_cols;
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
        wattron  (win->win, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
        wborder  (win->win, 0, 0, 0, 0, tl, tr, bl, br);
        wattroff (win->win, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
        refresh  ();
        wrefresh (win->win);

        // store corners
        win->win_border [0] = 0;
        win->win_border [1] = 0;
        win->win_border [2] = 0;
        win->win_border [3] = 0;
        win->win_border [4] = tl;
        win->win_border [5] = tr;
        win->win_border [6] = bl;
        win->win_border [7] = br;

        // next window
        win = win->win_next;

    } while (win != NULL);
}



/*
    Fix single ncurses border corner character
    ---------------
    Returns correct corner character int
    Called by fix_corners()

        y, x    - terminal screen coordinates

*/
static int fix_corner_char (int y, 
                            int x)
{
    int i;
    int ch;

    // get border character
    //
    //     curscr:
    //       - terminal or "physical" screen
    //       - i.e. allows access to every character currently on 
    //         screen without needing to know what window it is in
    //
    ch = mvwinch (curscr, y, x) & A_CHARTEXT;

    // if border character already corrected, return it as is
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

    // check for surrounding border characters
    //
    //      t           │                     
    //    l c r       ─ c   -->   {t, r, b, l}
    //      b                     {1, 0, 0, 1}  
    //
    int borders [4] = {0};
    int horiz_line  = ACS_HLINE & A_CHARTEXT;
    int vert_line   = ACS_VLINE & A_CHARTEXT;

        // top
    if (y > header_offset) {
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
    Render header titles
*/
static void render_header_titles (layout_t *layout)
{
    int row = 0;
    int ch;
    bool key_color_toggle;
    plugin_t* head_plugin = layout->plugins;
    plugin_t* curr_plugin;
    WINDOW* header;
    char *curr_title;
    int   header_title_indent = 0;
    int   title_str_len = scr_cols - (strlen (prog_title) + 4);
    char *titles_str = (char *) malloc (sizeof (char) * title_str_len);

    header = layout->header;

    extern int plugin_code_size;    // plugins/_plugins.c

        // clear titles string
    memset (titles_str, '\0', title_str_len);
    titles_str [0] = ' ';

    for (int j = 0; j < strlen (layout->hdr_key_str); j++) {

        ch = layout->hdr_key_str [j];
        
        // render row of plugin titles
        if (ch == '\n') {
            header_title_indent = scr_cols - strlen (titles_str) - 2;

            //render_header_title (header, row, header_title_indent, titles_str);

            key_color_toggle = false;

            // print title
            wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
            for (int i = 0; i < strlen(titles_str) + 1; i++) {

                mvwprintw (header, row, header_title_indent + i, "%c", titles_str[i]);

                // turn off key character color
                if (key_color_toggle) {
                    wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
                    key_color_toggle = false;
                }

                if (titles_str[i] == '(') {
                    wattron (header, COLOR_PAIR(TITLE_KEY_COLOR));
                    key_color_toggle = true;
                }
            }
            wrefresh  (header);

            memset (titles_str, '\0', title_str_len);
            row += 1;
            continue;
        }

        // get header title
        for (int k = 0; k < plugin_code_size; k++) {
            if (ch == layout->plugins->key) {
                curr_title = (char *) layout->plugins->title;
                break;
            } 
        }

        // get title
        curr_plugin = head_plugin;
        do {
            if (ch == curr_plugin->key) {
                curr_title = (char *)curr_plugin->title;
                goto title_found;
            }
            curr_plugin = curr_plugin->next;
        } while (curr_plugin != NULL);
        title_found:

        // concatenate title string
        strcat (titles_str, curr_title);
        strcat (titles_str, "  ");
    }
    wrefresh (header);
    free (titles_str);
}



/*
    Render window titles
*/
static void render_window_titles (layout_t *layout)
{
    char *title;
    window_t *curr_window = layout->windows;
    plugin_t* head_plugin = layout->plugins;

    do {

        // get title
        plugin_t* curr_plugin = head_plugin;
        do {
            if (curr_window->key == curr_plugin->key)
                title = (char *)curr_plugin->title;
            curr_plugin = curr_plugin->next;
        } while (curr_plugin != NULL);

        // render title
        int key_color_toggle = false;
        WINDOW* win = curr_window->win;

            // calculate indent
        int title_length = strlen (title);
        int title_indent = (curr_window->win_cols - title_length) / 2;

            // print title
        wattron (win, COLOR_PAIR(WINDOW_TITLE_COLOR));
        for (int i = 0; i < title_length + 1; i++) {

            mvwprintw (win, 0, title_indent - 1 + i, "%c", title[i]);
            wrefresh  (win);

            // turn off key character color
            if (key_color_toggle) {
                wattron (win, COLOR_PAIR(WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            // set key shortcut color  (c)
            if (title[i] == '(') {
                wattron (win, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }

        // next window
        curr_window = curr_window->win_next;

    } while (curr_window != NULL);
}

