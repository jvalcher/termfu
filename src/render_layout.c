
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

// main functions
static void    set_layout             (char*, state_t*, layout_t*);
static void    calculate_layout       (layout_t*);

#ifndef DEBUG
    static void    render_header          (state_t*);
    static void    render_windows         (state_t*);
    static void    render_header_titles   (state_t*);
    static WINDOW* allocate_window        (int, int, int, int);
    static WINDOW *render_window          (int, int, int, int);
    static void    render_titles          (state_t*);
    static void    fix_corners            (window_t*);
    static int     fix_corner_char        (int, int);
#endif

#ifdef DEBUG
    static void    print_layout_info      (state_t*);
#endif

char *prog_name    = "termIDE";
char *first_layout = "&F";

int   scr_rows;
int   scr_cols;
int   header_offset;



void 
render_layout (char    *label,
               state_t *state)
{
    set_layout (label, state, state->layouts);

    calculate_layout (state->curr_layout);

#ifdef DEBUG

    print_layout_info (state);

#endif
#ifndef DEBUG

    render_header (state);

    render_windows (state);

#endif
}



/****************
  Main functions
 ****************/


/*
   Find layout that matches label
*/
static void
set_layout (char     *label,
            state_t  *state,
            layout_t *layout)
{
    do {
        if (strcmp (label, first_layout) == 0 || strcmp (label, layout->label) == 0) {
            state->curr_layout = layout;
            break;
        } 
        layout = layout->next;
    } while (layout != NULL);

    // if label not found, use first layout
    if (layout == NULL) {
        state->curr_layout = state->layouts;
    }
}



/*
    Calculate layout dimentions
    ------------
    Calculate position, dimensions of plugin windows
    Allocate memory for window_t structs in state->layout->windows

    Parameters:

        win_rows   - main window height
        win_cols   - main window width
        layout     - layout_t struct
*/
static void
calculate_layout (layout_t *layout)
{
    // calculate cols, rows in window area below header
    scr_rows = getmaxy (stdscr);
    scr_cols = getmaxx (stdscr);
    header_offset = layout->num_hdr_key_rows + 1;
    int win_cols = scr_cols;
    int win_rows = scr_rows - header_offset;


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
                curr_window->selected = false;
                curr_window->file_offsets = NULL;
                curr_window->next = NULL;
                curr_window->out_file_path = NULL;

                // set head window or link previous
                if (y == 0 && x == 0) {
                    layout->windows = curr_window; // head
                } else {
                    prev_window->next = curr_window;    // link previous
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


#ifndef DEBUG

/*
    Render header WINDOW
    ---------

        <prog_name> : <curr_layout>           <header titles>
*/
static void
render_header (state_t *state)
{
#ifdef DEBUG
    (void) state;
#endif

#ifndef DEBUG
    layout_t *layout = state->curr_layout;
    int title_len = strlen (prog_name);
    char *colon = ":";

    // create window
    layout->header = allocate_window (header_offset, COLS, 0, 0);

    // print
        // program name
    wattron (layout->header, COLOR_PAIR(MAIN_TITLE_COLOR) | A_BOLD | A_UNDERLINE);
    mvwprintw (layout->header, 1, 2, "%s", prog_name);
    wattrset (layout->header, A_NORMAL);

        // colon
    wattron (layout->header, COLOR_PAIR(WHITE_BLACK));
    mvwprintw (layout->header, 1, title_len + 3, "%s", colon);

        // current layout
    wattron (layout->header, COLOR_PAIR(LAYOUT_TITLE_COLOR));
    mvwprintw (layout->header, 1, title_len + 4, "%s", layout->label);
    attrset (A_NORMAL);

        // header plugin titles
    render_header_titles (state);

    refresh();
    wrefresh (layout->header);
#endif
}



/*
    Render WINDOWs
*/
static void render_windows (state_t *state)
{
#ifdef DEBUG
    (void) state;
#endif

#ifndef DEBUG
    // render windows
    window_t *w = state->curr_layout->windows;
    do {
        w->win = render_window (w->win_rows,
                                w->win_cols,
                                w->win_y + header_offset,
                                w->win_x);
        w = w->next;
    } while (w != NULL);

    fix_corners (state->curr_layout->windows);

    render_titles (state);
#endif
}



/******************
  Helper functions
 ******************/


/*
    Render header plugin titles
    -------
    - Used by render_header()
*/
static void
render_header_titles (state_t *state)
{
    int row = 0;
    int ch;
    bool key_color_toggle;
    layout_t *layout = state->curr_layout;
    plugin_t* head_plugin = state->plugins;
    plugin_t* curr_plugin;
    WINDOW* header;
    char *curr_title;
    int   header_title_indent = 0;
    int   title_str_len = scr_cols - (strlen (prog_name) + 4);
    char *titles_str = (char *) malloc (sizeof (char) * title_str_len);

    header = layout->header;
    memset (titles_str, '\0', title_str_len);
    titles_str [0] = ' ';

    for (size_t j = 0; j < strlen (layout->hdr_key_str); j++) {

        ch = layout->hdr_key_str [j];
        
        // render row of plugin titles
        if (ch == '\n') {
            header_title_indent = scr_cols - strlen (titles_str) - 2;

            key_color_toggle = false;

            // print title
            wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
            for (size_t i = 0; i < strlen(titles_str) + 1; i++) {

                mvwprintw (header, row, header_title_indent + i, "%c", titles_str[i]);

                // alternate (_) character color
                if (key_color_toggle) {
                    wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
                    key_color_toggle = false;
                }
                if (titles_str[i] == '(') {
                    wattron (header, COLOR_PAIR(TITLE_KEY_COLOR));
                    key_color_toggle = true;
                }
            }

            memset (titles_str, '\0', title_str_len);
            row += 1;
            continue;
        }

        // get title from key
        curr_plugin = head_plugin;
        do {
            if (ch == curr_plugin->key) {
                curr_title = (char*)curr_plugin->title;
                break;
            }
            curr_plugin = curr_plugin->next;
        } while (curr_plugin != NULL);

        // append title to titles_str
        strcat (titles_str, curr_title);
        strcat (titles_str, "  ");
    }

    free (titles_str);
}



/*
    Create Ncurses window
    -------------------
    - Relative to stdscr

        rows, cols  - height, width
        y, x        - window position in terms of top left coordinate
*/
static WINDOW*
render_window (int rows, 
               int cols, 
               int y, 
               int x) 
{
    // create window object
    WINDOW *win = allocate_window (rows, cols, y, x);
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
    Fix broken border corners caused by overlapping in calculate_layout()
   
          │       │
         ─┐ -->  ─┤
          │       │
*/
static void
fix_corners (window_t *win)
{
    int of = header_offset;

    do {
        int y     = win->win_y;
        int x     = win->win_x;
        int rows  = win->win_rows;
        int cols  = win->win_cols;
        int tl = 0;     // top left
        int tr = 0;     // top right
        int bl = 0;     // bottom left
        int br = 0;     // bottom right

        tl = fix_corner_char (y + of, x);
        tr = fix_corner_char (y + of, x + (cols - 1));
        bl = fix_corner_char (y + (rows - 1) + of, x);
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

        win = win->next;
    } while (win != NULL);
}



/*
    Fix single ncurses border corner character
    ---------------
    - Returns correct corner character int
    - Called by fix_corners()

        y, x    - window top left coordinates on stdscr

*/
static int
fix_corner_char (int y, 
                 int x)
{
    int i;
    unsigned int uch;
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
    uch = 0;
    int corrected_chars [] = {
        ACS_LTEE,
        ACS_RTEE,
        ACS_BTEE,
        ACS_TTEE,
        ACS_PLUS
    };
    for (i = 0; i < 5; i++) {
        if (uch == (corrected_chars [i] & A_CHARTEXT)) {
            return corrected_chars [i];
        }
    }

    // check for surrounding border characters
    //
    //      t           │         borders[]
    //    l c r       ─ c   -->      ==
    //      b                      t, r, b, l   
    //                            {1, 0, 0, 1}
    //
    // TODO: make quicker with bitwise operations
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
    Render window titles
*/
static void 
render_titles (state_t *state)
{
    char *title;
    window_t *curr_window = state->curr_layout->windows;
    plugin_t* head_plugin = state->plugins;

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

        refresh();
        wrefresh(win);

        curr_window = curr_window->next;

    } while (curr_window != NULL);

}



/*
    Allocate new Ncurses window relative to stdscr
*/
static WINDOW* 
allocate_window (int rows,
                 int cols,
                 int y,
                 int x)
{
    WINDOW *win = newwin (rows, cols, y, x);
    if (win == NULL)
        pfeme ("Unable to create window\n");
    return win;
}

#endif


#ifdef DEBUG

/*
    Print layout, plugin information for debugging
    ---------
    - Data for first <n> layout(s)
    - Current layout's plugin key bindings
    - Called in render_layout.c -> render_layout()
*/
static void print_layout_info (state_t *state)
{
    int i, j;
    layout_t *layout = state->curr_layout;

    puts ("");
    printf ("Label: %s\n", layout->label);
    puts ("-----------");
    printf ("Window row ratio: %d\n", layout->row_ratio);
    printf ("Window col ratio: %d\n", layout->col_ratio);

    // header keys
    printf ("Header keys: \n%s\n", layout->hdr_key_str);
    puts ("");

    // window segement matrix
    printf ("Window segment matrix: \n");
    for (i = 0; i < layout->row_ratio; i++) {
        for (j = 0; j < layout->col_ratio; j++) {
            printf ("%c", layout->win_matrix [i][j]);
        }
        puts ("");
    }
    puts ("");

    // plugins
    printf ("Plugins:\n");
    plugin_t* p = state->plugins;
    do {
        printf ("%s : %c : %s\n", p->code, p->key, p->title);
        p = p->next;
    } while (p != NULL);
    puts ("");
}

#endif

