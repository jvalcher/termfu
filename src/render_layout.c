
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

static void      allocate_windows      (state_t *state);
static void      free_window_data      (state_t*);
static layout_t *get_label_layout      (char*, layout_t*);
static void      calculate_layout      (layout_t*, state_t*);

#ifndef DEBUG
    static void      render_header         (layout_t*, state_t*);
    static void      render_windows        (state_t*);
    static void      render_header_titles  (layout_t*, state_t*);
    static WINDOW   *allocate_window       (int, int, int, int);
    static void      render_window         (window_t*);
    static void      render_window_titles  (state_t*);
    static void      fix_corners           (state_t*);
    static int       fix_corner_char       (int, int);
#endif

#ifdef DEBUG
    static void      print_layout_info     (layout_t*);
#endif

bool  window_t_arr_is_not_allocated = true;
int   scr_rows;
int   scr_cols;
int   header_offset;



void
render_layout (char     *label,
               state_t  *state)
{
    layout_t *layout;

    if (window_t_arr_is_not_allocated) {
        allocate_windows (state);
        window_t_arr_is_not_allocated = false;
    } else {
        free_window_data (state);
    }

    layout = get_label_layout (label, state->layouts);

    calculate_layout (layout, state);

#ifdef DEBUG

    print_layout_info (layout);

#endif

#ifndef DEBUG

    render_header  (layout, state);

    render_windows (state);

#endif
}



static void
allocate_windows (state_t *state)
{
    state->windows = (window_t**) malloc (state->num_plugins * sizeof (window_t));
    for (int i = 0; i < state->num_plugins; i++) {
        state->windows [i] = NULL;
    }
}



static void
free_window_data (state_t *state)
{
    for (int i = 0; i < state->num_plugins; i++) {
        if (state->windows [i]) {
            delwin (state->windows[i]->WIN);
        }
    }
}



/*
   Find layout that matches label
*/
static layout_t*
get_label_layout (char     *label,
                   layout_t *layouts)
{
    do {
        if (strcmp (label, FIRST_LAYOUT)   == 0 || 
            strcmp (label, layouts->label) == 0) {
            return layouts;
        } 
        layouts = layouts->next;
    } while (layouts != NULL);
    pfeme ("Layout \"%s\" not found", label);
}



/*
    Calculate layout dimentions
    ------------
    - Calculate position, dimensions in window_t
    - Allocate window_t structs if needed
*/
static void
calculate_layout (layout_t *layout,
                  state_t *state)
{
    int        win_cols,
               win_rows,
               i, j,
               y, x,
               yc, xc,
               xi, yi,
               row_ratio,
               col_ratio,
               floor_segm_rows,
               floor_segm_cols,
               segm_row_rem,
               segm_col_rem,
               col_rem,
               segm_rows        [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0},
               segm_cols        [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0},
               segm_ys          [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0},
               segm_xs          [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0},
               used_segm_matrix [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS] = {0},
               plugin_index;
    char       key,
             **layout_matrix;
    window_t  *curr_window = NULL;



    // calculate cols, rows in window area below header
    scr_rows = getmaxy (stdscr);        // TODO: Ncurses not initialized for DEBUG run
    scr_cols = getmaxx (stdscr);
    header_offset = layout->num_hdr_key_rows + 1;
    win_cols = scr_cols;
    win_rows = scr_rows - header_offset;


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
    row_ratio = layout->row_ratio;
    col_ratio = layout->col_ratio;
    floor_segm_rows = win_rows / row_ratio;  // floor row amount per segment
    floor_segm_cols = win_cols / col_ratio;

        // calculate total row, column remainders
    segm_row_rem = win_rows - (floor_segm_rows * row_ratio);
    segm_col_rem = win_cols - (floor_segm_cols * col_ratio);

        // add base segment rows, columns, distribute remainders
    for (y = 0; y < row_ratio; y++) {
        col_rem = segm_col_rem;
        for (x = 0; x < col_ratio; x++) {
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
    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {
            yc = 0;
            while (yc < y) {
                segm_ys [y][x] += segm_rows [yc][x];
                yc += 1;
            }
            xc = 0;
            while (xc < x) {
                segm_xs [y][x] += segm_cols [y][xc];
                xc += 1;
            }
        }
    }

    layout_matrix = (char**) layout->win_matrix;

    // create windows
    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {

            // if unused segment
            if (used_segm_matrix [y][x] == 0) {

                key = layout_matrix [y][x];

                plugin_index = state->plugin_key_index [(int)key];

                // get or allocate window_t struct
                if (state->windows[plugin_index]) {
                    curr_window = state->windows[plugin_index];
                } else {
                    curr_window = (window_t *) malloc (sizeof (window_t));
                    if (curr_window == NULL) {
                        pfeme ("Unable to allocate memory for window_t struct");
                    }
                    state->windows [plugin_index] = curr_window;
                }

                curr_window->selected       = false;
                curr_window->file_offsets   = NULL;

                // calculate rows, cols
                //           
                //         ┌──a
                //         │ssa
                //         │ssc
                //         bbww
                //
                    // rows
                yi = y;
                curr_window->win_rows = 0;
                while (yi < row_ratio) {
                    if (layout_matrix [yi][x] == key) {
                        curr_window->win_rows += segm_rows [yi][x];
                        yi += 1;
                    } else {
                        break;
                    }
                }
                    // cols
                xi = x;
                curr_window->win_cols = 0;
                while (xi < col_ratio) {
                    if (layout_matrix [y][xi] == key) {
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
                for (i = y; i < yi; i++) {
                    for (j = x; j < xi; j++) {
                        used_segm_matrix [i][j] = 1;
                    }
                }

#ifdef DEBUG
                puts ("");
                printf ("WINDOW: %s (%c)\n", state->plugins[plugin_index]->title, state->plugins[plugin_index]->key);
                puts   ("------------");
                printf ("win_x: %d\n", curr_window->win_x);
                printf ("win_cols: %d\n", curr_window->win_cols);
                printf ("win_y: %d\n", curr_window->win_y);
                printf ("win_rows: %d\n", curr_window->win_rows);
                puts   ("");
#endif
            }
        }
    }
}


#ifndef DEBUG


/*
    Render header WINDOW
    ---------

        <program name> : <current layout>           <header titles>
*/
static void
render_header (layout_t *layout, 
               state_t *state)
{
#ifdef DEBUG
    (void) state;
#endif

#ifndef DEBUG

    WINDOW *header = allocate_window (header_offset, COLS, 0, 0);
    int title_len  = strlen (PROGRAM_NAME);

    // print
        // program name
    wattron (header, COLOR_PAIR(MAIN_TITLE_COLOR) | A_BOLD | A_UNDERLINE);
    mvwprintw (header, 1, 2, "%s", PROGRAM_NAME);
    wattrset (header, A_NORMAL);

        // colon
    wattron (header, COLOR_PAIR(WHITE_BLACK));
    mvwprintw (header, 1, title_len + 3, "%s", ":");

        // current layout
    wattron (header, COLOR_PAIR(LAYOUT_TITLE_COLOR));
    mvwprintw (header, 1, title_len + 4, "%s", layout->label);
    attrset (A_NORMAL);

    render_header_titles (layout, state);

    refresh();
    wrefresh (header);

    state->header = header;

#endif
}



/*
    Render WINDOWs
*/
static void 
render_windows (state_t *state)
{
#ifdef DEBUG
    (void) state;
#endif

#ifndef DEBUG
    int       i;

    for (i = 0; i < state->num_plugins; i++) {
        if (state->windows [i]) {
            render_window (state->windows [i]);
        } else {
            state->windows[i]->WIN = NULL;
        }
    }

    fix_corners (state);

    render_window_titles (state);

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
render_header_titles (layout_t *layout,
                      state_t *state)
{
    int       row, key, 
              header_title_indent,
              title_str_len,
              title_str_ch_left;
    bool      key_color_toggle;
    WINDOW   *header;
    char     *titles_str,
             *curr_title;

    row                 = 0;
    header_title_indent = 0;
    title_str_len       = scr_cols - (strlen (PROGRAM_NAME) + 4);
    title_str_ch_left   = title_str_len;
    titles_str          = (char*) malloc (sizeof (char) * title_str_len);
    header              = state->header;

    memset (titles_str, '\0', title_str_len);
    titles_str [0]      = ' ';

    for (size_t j = 0; j < strlen (layout->hdr_key_str); j++) {

        key = layout->hdr_key_str [j];
        
        // render row of plugin titles
        if (key == '\n') {

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


        // append title to titles_str
        curr_title         = state->plugins[state->plugin_key_index [key]]->title;
        title_str_ch_left -= strlen (curr_title) + 2;
            //
        if (title_str_ch_left < 1) {
            pfeme ("To many header titles in row %d", row + 1);
        } else {
            strcat (titles_str, curr_title);
            strcat (titles_str, "  ");
        }
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
static void
render_window (window_t *win)
{
    // create window object
    win->WIN = allocate_window (win->win_rows, 
                                win->win_cols, 
                                win->win_y + header_offset, 
                                win->win_x);

    if (win->WIN == NULL)
        pfeme  ("Unable to create window\n");

    // render border
    wattron  (win->WIN, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
    wborder  (win->WIN, 0,0,0,0,0,0,0,0);
    wattroff (win->WIN, COLOR_PAIR(BORDER_COLOR) | A_BOLD);

    refresh ();
    wrefresh (win->WIN);
}



/*
    Fix broken border corners caused by overlapping in calculate_layout()
   
          │       │
         ─┐ -->  ─┤
          │       │
*/
static void
fix_corners (state_t *state)
{
    int       i, y, x,
              rows, cols,
              tl, tr, bl, br,
              of = header_offset;
    window_t **wins = state->windows;
    window_t  *win;

    for (i = 0; i < state->num_plugins; i++) {

        if (wins [i]) {
            win = wins[i];
        } else {
            continue;
        }

        y    = win->win_y;
        x    = win->win_x;
        rows = win->win_rows;
        cols = win->win_cols;
        tl   = 0;     // top left
        tr   = 0;     // top right
        bl   = 0;     // bottom left
        br   = 0;     // bottom right

        tl = fix_corner_char (y + of, x);
        tr = fix_corner_char (y + of, x + (cols - 1));
        bl = fix_corner_char (y + (rows - 1) + of, x);
        br = fix_corner_char (y + (rows - 1) + of, x + (cols - 1));

        // set corners
        //
        //      wborder (win, ls, rs, ts, bs, tl, tr, bl, br)
        //
        wattron  (win->WIN, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
        wborder  (win->WIN, 0, 0, 0, 0, tl, tr, bl, br);
        wattroff (win->WIN, COLOR_PAIR(BORDER_COLOR) | A_BOLD);

        refresh  ();
        wrefresh (win->WIN);

        // store corners
        win->win_border [0] = 0;
        win->win_border [1] = 0;
        win->win_border [2] = 0;
        win->win_border [3] = 0;
        win->win_border [4] = tl;
        win->win_border [5] = tr;
        win->win_border [6] = bl;
        win->win_border [7] = br;

    }
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
render_window_titles (state_t *state)
{
    int       i,
              title_length,
              title_indent;
    char     *title;
    bool      key_color_toggle;
    WINDOW   *Win;

    for (i = 0; i < state->num_plugins; i++) {

        if (state->windows [i]) {
            Win = state->windows[i]->WIN;
        } else {
            continue;
        }

        title = state->plugins[i]->title;
        key_color_toggle = false;

            // calculate indent
        title_length =  strlen (title);
        title_indent = (state->windows[i]->win_cols - title_length) / 2;

            // print title
        wattron (Win, COLOR_PAIR(WINDOW_TITLE_COLOR));
        for (int i = 0; i < title_length + 1; i++) {

            mvwprintw (Win, 0, title_indent - 1 + i, "%c", title[i]);
            wrefresh  (Win);

            // turn off key character color
            if (key_color_toggle) {
                wattron (Win, COLOR_PAIR(WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            // set key shortcut color  (c)
            if (title[i] == '(') {
                wattron (Win, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }

        refresh();
        wrefresh(Win);
    }
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

#endif  //  #ifndef DEBUG



#ifdef DEBUG

/*
    Print layout, plugin information for debugging
    ---------
    - Data for first <n> layout(s)
    - Current layout's plugin key bindings
    - Called in render_layout.c -> render_layout()
*/
static void print_layout_info (layout_t *layout)
{
    int i, j;

    // layouts
    puts   ("");
    printf ("LAYOUT: %s\n", layout->label);
    puts   ("------------");
    puts   ("");
    
    printf ("Window row ratio: %d\n", layout->row_ratio);
    printf ("Window col ratio: %d\n", layout->col_ratio);
    printf ("Header keys: \n\n%s\n", layout->hdr_key_str);
    printf ("Window segment matrix: \n");
    for (i = 0; i < layout->row_ratio; i++) {
        for (j = 0; j < layout->col_ratio; j++) {
            printf ("%c", layout->win_matrix [i][j]);
        }
        puts ("");
    }
    puts ("");
}

#endif
