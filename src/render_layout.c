
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>
#include <stdbool.h>
#include <ncurses.h>

#include "render_layout.h"
#include "data.h"
#include "utilities.h"

static void      free_nc_window_data   (state_t*);
static layout_t *get_label_layout      (char*, layout_t*);
static void      calculate_layout      (layout_t*, state_t*);
static void      render_header         (layout_t*, state_t*);
static void      render_windows        (state_t*);
static void      render_header_titles  (layout_t*, state_t*);
static void      render_window         (window_t*);
static void      render_window_titles  (state_t*);
static void      fix_corners           (state_t*);
static int       fix_corner_char       (int, int);

bool  first_window_t_allocation = true;
int   scr_rows;
int   scr_cols;
int   header_offset;



void
render_layout (char     *label,
               state_t  *state)
{
    if (state->curr_layout != NULL) {
        free_nc_window_data (state);
    }

    state->curr_layout = get_label_layout (label, state->layouts);

    calculate_layout (state->curr_layout, state);

    render_header (state->curr_layout, state);

    render_windows (state);
}



static void
free_nc_window_data (state_t *state)
{
    for (int i = 0; i < state->num_plugins; i++) {

        if (state->plugins[i]->has_window) {

            // Ncurses WINDOWs
            if (state->plugins[i]->win->IWIN) {
                delwin (state->plugins[i]->win->IWIN);
            }
            if (state->plugins[i]->win->DWIN) {
                delwin (state->plugins[i]->win->DWIN);
            }
            if (state->plugins[i]->win->WIN) {
                delwin (state->plugins[i]->win->WIN);
            }
            refresh ();

            state->plugins[i]->has_window = false;
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

    pfeme ("Layout \"%s\" not found\n", label);
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

    // Calculate cols, rows in window area below header
    scr_rows = getmaxy (stdscr);
    scr_cols = getmaxx (stdscr);
    header_offset = layout->num_hdr_key_rows + 1;
    win_cols = scr_cols;
    win_rows = scr_rows - header_offset;


    // Create matrices for number of rows, columns per segment
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

        // Calculate total row, column remainders
    segm_row_rem = win_rows - (floor_segm_rows * row_ratio);
    segm_col_rem = win_cols - (floor_segm_cols * col_ratio);

        // Add base segment rows, columns, distribute remainders
    for (y = 0; y < row_ratio; y++) {
        col_rem = segm_col_rem;
        for (x = 0; x < col_ratio; x++) {
            segm_rows [y][x] = floor_segm_rows + (segm_row_rem > 0 ? 1 : 0);
            segm_cols [y][x] = floor_segm_cols + (col_rem > 0      ? 1 : 0);
            col_rem -= 1;
        }
        segm_row_rem -= 1;
    }

    // Create matrices for top left y and x segment coordinates
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

    // clear window statuses
    for (i = 0; i < state->num_plugins; i++) {
        if (state->plugins[i]->win != NULL) {
            state->plugins[i]->has_window = false;
        }
    }

    // Set window_t structs data
    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {

            // If unused segment
            if (used_segm_matrix [y][x] == 0) {

                key = layout_matrix [y][x];

                plugin_index = state->plugin_key_index [(int)key];

                state->plugins[plugin_index]->has_window = true;

                curr_window = state->plugins[plugin_index]->win;
                if (curr_window == NULL) {
                    pfeme ("No window allocated for plugin %s (code: %s, index: %d)\n",
                            state->plugins[plugin_index]->title,
                            state->plugins[plugin_index]->code,
                            plugin_index);
                }

                // Calculate rows, cols
                //           
                //         ┌──a
                //         │ssa
                //         │ssc
                //         bbww
                //
                    // rows
                yi = y;
                curr_window->rows = 0;
                while (yi < row_ratio) {
                    if (layout_matrix [yi][x] == key) {
                        curr_window->rows += segm_rows [yi][x];
                        yi += 1;
                    } else {
                        break;
                    }
                }
                    // cols
                xi = x;
                curr_window->cols = 0;
                while (xi < col_ratio) {
                    if (layout_matrix [y][xi] == key) {
                        curr_window->cols += segm_cols [y][xi];
                        xi += 1;
                    } else {
                        break;
                    }
                }

                // Set top left coordinates
                curr_window->y = segm_ys [y][x];
                curr_window->x = segm_xs [y][x];

                // Overlap borders
                if (y > 0) {
                    curr_window->y -= 1;
                    curr_window->rows += 1;
                }
                if (x > 0) {
                    curr_window->x -= 1;
                    curr_window->cols += 1;
                }

                // Set used segments
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
            }
        }
    }
}



/*
    Render header WINDOW
    ---------

        <program name> : <current layout>           <header titles>
*/
static void
render_header (layout_t *layout, 
               state_t *state)
{
    WINDOW *header;

    state->header = newwin (header_offset, COLS, 0, 0);
    if (state->header == NULL) {
        pfeme ("Unable to create header window for layout \"%s\"\n", layout->label);
    }
    header = state->header;
    int title_len  = strlen (PROGRAM_NAME);

    // program name
    wattron (header, COLOR_PAIR(MAIN_TITLE_COLOR) | A_BOLD);
    mvwprintw (header, 1, 2, "%s", PROGRAM_NAME);
    wattrset (header, A_NORMAL);

    // colon
    wattron (header, COLOR_PAIR(WHITE_BLACK));
    mvwprintw (header, 1, title_len + 3, "%s", ":");

    // current layout
    wattron (header, COLOR_PAIR(LAYOUT_TITLE_COLOR));
    mvwprintw (header, 1, title_len + 4, "%s", layout->label);
    attrset (A_NORMAL);

    refresh ();
    wrefresh (header);

    render_header_titles (layout, state);
}



/*
    Render Ncurses WINDOWs
*/
static void 
render_windows (state_t *state)
{
    int i;

    for (i = 0; i < state->num_plugins; i++) {
        if (state->plugins[i]->has_window) {
            render_window (state->plugins[i]->win);
        } 
    }

    fix_corners (state);

    render_window_titles (state);
}



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

            titles_str[0] = '\0';
            row += 1;
            continue;
        }


        // append title to titles_str
        curr_title         = state->plugins[state->plugin_key_index [key]]->title;
        title_str_ch_left -= strlen (curr_title) + 2;
            //
        if (title_str_ch_left < 1) {
            pfeme ("To many header titles in row %d\n", row + 1);
        } else {
            strcat (titles_str, curr_title);
            strcat (titles_str, "  ");
        }
    }

    wrefresh (header);
    free (titles_str);
}



/*
    Create Ncurses window
*/
static void
render_window (window_t *win)
{
    int left_spaces, right_spaces;

    // parent window
    win->WIN = newwin (win->rows, 
                       win->cols, 
                       win->y + header_offset, 
                       win->x);
    if (win == NULL) {
        pfeme ("Unable to create window \"%s\" (rows: %d, cols: %d, y: %d, x: %d)\n",
                    win->code, win->rows, win->cols, win->y, win->x);
    }
    wattron  (win->WIN, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
    wborder  (win->WIN, 0,0,0,0,0,0,0,0);
    wattroff (win->WIN, COLOR_PAIR(BORDER_COLOR) | A_BOLD);
    wrefresh (win->WIN);

    // topbar subwindow
    win->topbar_rows = 0;
    win->topbar_y = 0;
    win->topbar_cols = 0;
    win->topbar_x = 0;

    if (win->has_topbar) {

        win->topbar_rows = 1;
        win->topbar_cols = win->cols - 2;
        win->topbar_y = 1;
        win->topbar_x = 1;

        win->IWIN = derwin (win->WIN,
                            win->topbar_rows,
                            win->topbar_cols,
                            win->topbar_y,
                            win->topbar_x);
        if (win->IWIN == NULL) {
            pfeme  ("Unable to create topbar subwindow for \"%s\" (rows: %d, cols: %d, y: %d, x: %d)\n",
                            win->code,
                            win->topbar_rows,
                            win->topbar_cols,
                            win->topbar_y,
                            win->topbar_x);
        }

        left_spaces = (win->topbar_cols - strlen(win->topbar_title)) / 2;
        right_spaces = win->topbar_cols - strlen(win->topbar_title) - left_spaces;
        left_spaces = left_spaces > 0 ? left_spaces : 0;
        right_spaces = right_spaces > 0 ? left_spaces : 0;

        wattron   (win->IWIN, COLOR_PAIR(WINDOW_INPUT_TITLE_COLOR));
        mvwprintw (win->IWIN, 0, 0, "%*c%.*s%*c", left_spaces, ' ',
                                                  win->topbar_cols, win->topbar_title,
                                                  right_spaces, ' ');
        wattroff  (win->IWIN, COLOR_PAIR(WINDOW_INPUT_TITLE_COLOR));
        wrefresh  (win->IWIN);
    }

    // data subwindow
    win->data_win_rows = win->rows - win->topbar_rows - 2;
    win->data_win_cols = win->cols - 2;
    win->data_win_y = win->topbar_y + 1;
    win->data_win_x = 1;

    win->DWIN = derwin (win->WIN,
                        win->data_win_rows,
                        win->data_win_cols,
                        win->data_win_y,
                        win->data_win_x);
    if (win->DWIN == NULL) {
        pfeme  ("Unable to create data subwindow for \"%s\" (rows: %d, cols: %d, y: %d, x: %d)\n",
                        win->code,
                        win->data_win_rows,
                        win->data_win_cols,
                        win->data_win_y,
                        win->data_win_x);
    }

    wrefresh (win->DWIN);
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
    window_t  *win;

    for (i = 0; i < state->num_plugins; i++) {

        if (state->plugins[i]->has_window) {
            win = state->plugins[i]->win;
        } else {
            continue;
        }

        y    = win->y;
        x    = win->x;
        rows = win->rows;
        cols = win->cols;
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
        win->border [0] = 0;
        win->border [1] = 0;
        win->border [2] = 0;
        win->border [3] = 0;
        win->border [4] = tl;
        win->border [5] = tr;
        win->border [6] = bl;
        win->border [7] = br;
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

        if (state->plugins[i]->has_window) {
            Win = state->plugins[i]->win->WIN;
        } else {
            continue;
        }

        title = state->plugins[i]->title;
        key_color_toggle = false;

            // calculate indent
        title_length =  strlen (title);
        title_indent = (state->plugins[i]->win->cols - title_length) / 2;

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

