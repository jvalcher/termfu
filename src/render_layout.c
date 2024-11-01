
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>
#include <stdbool.h>
#include <ncurses.h>
#include <errno.h>

#include "render_layout.h"
#include "data.h"
#include "utilities.h"

static layout_t *get_label_layout      (char*, layout_t*);
static int       calculate_layout      (layout_t*, state_t*);
static int       render_header         (layout_t*, state_t*);
static int       render_windows        (state_t*);

int scr_rows,
    scr_cols,
    header_offset;


// TODO: re-render layout on terminal screen size change

int
render_layout (char     *label,
               state_t  *state)
{
    int ret;

    if (state->curr_layout != NULL) {
        ret = free_nc_window_data (state);
        if (ret == FAIL) {
            pfem ("Failed to free Ncurses window data for layout \"%s\"",
                        state->curr_layout->label);
            goto render_layout_err;
        }
    }

    if ((state->curr_layout = get_label_layout (label, state->layouts)) == NULL) {
        pfem ("Failed to find layout");
        goto render_layout_err;
    }

    ret = calculate_layout (state->curr_layout, state);
    if (ret == FAIL) {
        pfem ("Failed to calculate layout \"%s\"", state->curr_layout->label);
        goto render_layout_err;
    }

    ret = render_header (state->curr_layout, state);
    if (ret == FAIL) {
        pfem ("Failed to render header for layout \"%s\"", state->curr_layout->label);
        goto render_layout_err;
    }

    ret = render_windows (state);
    if (ret == FAIL) {
        pfem ("Failed to render windows for layout \"%s\"", state->curr_layout->label);
        goto render_layout_err;
    }

    return A_OK;

render_layout_err:
    pemr ("Current layout: \"%s\", number of plugins: %d",
                state->curr_layout->label, state->num_plugins);
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

    pfem ("Layout \"%s\" not found", label);
    return NULL;
}



/*
    Calculate layout dimentions
    ------------
    - Calculate position, dimensions in window_t
    - Allocate window_t structs if needed
*/
static int
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
               segm_rows        [LAYOUT_ROW_SEGMENTS][LAYOUT_COL_SEGMENTS] = {0},
               segm_cols        [LAYOUT_ROW_SEGMENTS][LAYOUT_COL_SEGMENTS] = {0},
               segm_ys          [LAYOUT_ROW_SEGMENTS][LAYOUT_COL_SEGMENTS] = {0},
               segm_xs          [LAYOUT_ROW_SEGMENTS][LAYOUT_COL_SEGMENTS] = {0},
               used_segm_matrix [LAYOUT_ROW_SEGMENTS][LAYOUT_COL_SEGMENTS] = {0},
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

    // clear has_window statuses
    for (i = 0; i < state->num_plugins; i++) {
        state->plugins[i]->has_window = false;
    }


    // set window_t structs data
    layout_matrix = (char**) layout->win_matrix;
        //
    for (y = 0; y < row_ratio; y++) {
        for (x = 0; x < col_ratio; x++) {

            // if segment not processed
            if (used_segm_matrix [y][x] == 0) {

                key          = layout_matrix [y][x];
                plugin_index = state->plugin_key_index [(int)key];

                state->plugins[plugin_index]->has_window = true;

                if ((curr_window = state->plugins[plugin_index]->win) == NULL) {
                    pfemr ("No window allocated for plugin \"%s\" (index: %d, code: %s)\n",
                            state->plugins[plugin_index]->title,
                            plugin_index,
                            state->plugins[plugin_index]->code);
                }

                // calculate rows, cols
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

                // overlap borders
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
                for (i = y; i < yi; i++) {
                    for (j = x; j < xi; j++) {
                        used_segm_matrix [i][j] = 1;
                    }
                }
            }
        }
    }

    return A_OK;
}



/*
    Render header plugin titles
*/
static int
render_header_titles (layout_t *layout,
                      state_t *state)
{
    size_t    i, j;
    int       row, ch, 
              header_title_indent,
              title_row_len,
              title_row_rem_chars,
              prog_lay_pad_len,
              right_pad_len;
    bool      key_color_toggle;
    WINDOW   *header;
    char     *row_titles,
             *curr_title,
             *title_padding;

    header              = state->header;

    // set max header titles row length
    prog_lay_pad_len    = strlen ("  :   ");    // "_termfu_:_layout1__"
    title_row_len       = scr_cols - (strlen (PROGRAM_NAME) +
                                      strlen (state->curr_layout->label) +
                                      prog_lay_pad_len);
    title_row_rem_chars = title_row_len;

    // create title padding string
    right_pad_len = 2;                    
    if ((title_padding = (char*) malloc (right_pad_len + 1)) == NULL) {
        pfem ("malloc: %s", strerror (errno));
        pemr ("Failed to allocate header title padding string (len: %d)", right_pad_len);
    }
    for (i = 0; i < (size_t) right_pad_len; i++) {
        title_padding[i] = ' ';
    }
    title_padding [i] = '\0';

    // create header row titles buffer
    if ((row_titles = (char*) malloc (title_row_len + 1)) == NULL) {
        pfem ("malloc: %s", strerror (errno));
        pemr ("Failed to allocate header row string (len: %d)", title_row_len);
    }
    row_titles [0] = '\0';

    // parse hdr_key_str into header title rows
    row                 = 0;
    header_title_indent = 0;
    wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
        //
    for (i = 0; i < strlen (layout->hdr_key_str); i++) {

        ch = layout->hdr_key_str [i];
        
        // render row of plugin titles
        if (ch == '\n') {

            header_title_indent = scr_cols - strlen (row_titles) - right_pad_len;
            key_color_toggle    = false;

            // print title
            for (j = 0; j < strlen (row_titles) + 1; j++) {

                mvwprintw (header, row, header_title_indent + j, "%c", row_titles [j]);

                // change parenthesized character's color in t(i)tle
                if (key_color_toggle) {
                    wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
                    key_color_toggle = false;
                }
                else if (row_titles [j] == '(') {
                    wattron (header, COLOR_PAIR(TITLE_KEY_COLOR));
                    key_color_toggle = true;
                }
            }

            ++row;
            row_titles [0]      = '\0';
            title_row_rem_chars = title_row_len;
        }

        // append title to header row string
        else {

            curr_title = state->plugins[state->plugin_key_index [ch]]->title;

            // check if space for title
            title_row_rem_chars -= (strlen (curr_title) + right_pad_len);
            if (title_row_rem_chars < 1) {
                pfem ("To many header titles in row %d", row + 1);
                pem  ("Current title:      \"%s\"", curr_title);
                pemr ("Current row string: \"%s\"", row_titles);
            } 

            // append title
            else {
                strcat (row_titles, curr_title);
                strcat (row_titles, title_padding);
            }
        }
    }

    free (row_titles);
    free (title_padding);

    wrefresh (header);

    return A_OK;
}



/*
    Render header WINDOW
    ---------

        <program name> : <current layout>           <header titles>
*/
static int
render_header (layout_t *layout, 
               state_t  *state)
{
    int ret;
    WINDOW *header;

    if ((state->header = newwin (header_offset, COLS, 0, 0)) == NULL) {
        pfemr ("Failed to create header window (header_offset: %d, COLS: %d)",
                header_offset, COLS);
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

    // header titles
    ret = render_header_titles (layout, state);
    if (ret == FAIL) {
        pfemr ("Failed to render header titles");
    }

    return A_OK;
}



/*
    Create Ncurses window
    -------
    Used by render_windows()
*/
static int
render_window (window_t *win)
{
    int left_spaces, right_spaces;

    // parent window
    if ((win->WIN = newwin (win->rows, 
                       win->cols, 
                       win->y + header_offset, 
                       win->x)) == NULL)
    {
        pfemr ("Unable to create window \"%s\" (rows: %d, cols: %d, y: %d, x: %d)\n",
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

        if ((win->TWIN = derwin (win->WIN,
                            win->topbar_rows,
                            win->topbar_cols,
                            win->topbar_y,
                            win->topbar_x)) == NULL)
        {
            pfemr  ("Unable to create topbar subwindow for \"%s\" (rows: %d, cols: %d, y: %d, x: %d)\n",
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

        wattron   (win->TWIN, COLOR_PAIR(TOPBAR_COLOR) | A_BOLD);
        mvwprintw (win->TWIN, 0, 0, "%*c%.*s%*c", left_spaces, ' ',
                                                  win->topbar_cols, win->topbar_title,
                                                  right_spaces, ' ');
        wattroff  (win->TWIN, COLOR_PAIR(TOPBAR_COLOR) | A_BOLD);
        wrefresh  (win->TWIN);
    }

    // data subwindow
    win->data_win_rows = win->rows - win->topbar_rows - 2;
    win->data_win_cols = win->cols - 2;
    win->data_win_y = win->topbar_y + 1;
    win->data_win_x = 1;

    if ((win->DWIN = derwin (win->WIN,
                        win->data_win_rows,
                        win->data_win_cols,
                        win->data_win_y,
                        win->data_win_x)) == NULL)
    {
        pfemr  ("Unable to create data subwindow for \"%s\" (rows: %d, cols: %d, y: %d, x: %d)\n",
                        win->code,
                        win->data_win_rows,
                        win->data_win_cols,
                        win->data_win_y,
                        win->data_win_x);
    }

    wrefresh (win->DWIN);

    return A_OK;
}



/*
    Fix single ncurses border corner character
    ---------------
    - Called by fix_corners()

        y, x    - window top left coordinates on stdscr

    - Returns correct corner character int or 0 if character matches
      one that has already been fixed
*/
static int
fix_corner_char (int y, 
                 int x)
{
    int ch,
        borders [4] = {0},
        horiz_line,
        vert_line;

    // get border character
    ch = mvwinch (curscr, y, x) & A_CHARTEXT;

    // check for surrounding border characters
    //
    //      t           │         borders[]
    //    l c r       ─ c   -->      ==
    //      b                      t, r, b, l   
    //                            {1, 0, 0, 1}
    //
    horiz_line  = ACS_HLINE & A_CHARTEXT;
    vert_line   = ACS_VLINE & A_CHARTEXT;

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


// FIX: some fixed corner characters redrawn by wborder() 
// See misc/layout1.png

/*
    Fix broken border corners caused by overlapping in calculate_layout()
   
          │       │
         ─┐ -->  ─┤
          │       │
*/
static int
fix_corners (state_t *state)
{
    int       i, y, x,
              rows, cols,
              tl, tr, bl, br,
              of;
    window_t *win;

    for (i = 0; i < state->num_plugins; i++) {

        if (state->plugins[i]->has_window) {
            win = state->plugins[i]->win;
        } else {
            continue;
        }

        of   = header_offset;
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

    return A_OK;
}



/*
    Render window titles
*/
static int
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
            if ((Win = state->plugins[i]->win->WIN) == NULL) {
                pfemr ("Window not allocated for \"%s\" (key: %c, code: %s)",
                            state->plugins[i]->title,
                            state->plugins[i]->key,
                            state->plugins[i]->code);
            }
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

    return A_OK;
}



/*
    Render Ncurses WINDOWs
*/
static int
render_windows (state_t *state)
{
    int i, ret;

    for (i = 0; i < state->num_plugins; i++) {
        if (state->plugins[i]->has_window) {
            ret = render_window (state->plugins[i]->win);
            if (ret == FAIL) {
                pfemr ("Failed to render window \"%s\" (key: %c, code: %s)",
                            state->plugins[i]->title,
                            state->plugins[i]->key,
                            state->plugins[i]->code);
            }
        } 
    }

    ret = fix_corners (state);
    if (ret == FAIL) {
        pfemr ("Failed to fix window corners");
    }

    ret = render_window_titles (state);
    if (ret == FAIL) {
        pfemr ("Failed to render window titles");
    }

    return A_OK;
}



