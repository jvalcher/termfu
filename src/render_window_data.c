
#include "render_window_data.h"
#include "data.h"
#include "utilities.h"

static void  render_data                (window_t*, int);
static void  pulse_header_title_color   (state_t*, int);
       void  select_window              (state_t*);
       void  deselect_window            (void);
static bool  find_window_string         (WINDOW*, char*, int*, int*);

char     *curr_title = NULL;
window_t *curr_win   = NULL;
WINDOW   *curr_WIN   = NULL;
 


/*
    Render window changes
*/
void 
render_window_data (window_t *win, 
                    state_t  *state,
                    int       key,
                    int       render_type) 
{
    // CURRENT: pulse header string, select, deselect
    // CURRENT: fix window select loop

    switch (render_type) {

        case HEADER_TITLE_COLOR_ON:
            pulse_header_title_color (state, HEADER_TITLE_COLOR_ON);
            break;

        case HEADER_TITLE_COLOR_OFF:
            pulse_header_title_color (state, HEADER_TITLE_COLOR_OFF);
            break;

        case WINDOW_SELECT:
            select_window (state);
            break;

        case WINDOW_UNSELECT:
            deselect_window ();
            break;

        case WINDOW_DATA:
            render_data (win, key);
            break;

        // TODO:
        // --------
        // - Popup window
        // - Replace/modify (?) curr window with value entry field (break/watchpoints)
    }
}



/*
    Display file lines in Ncurses WINDOW shifted according to key
    ---------
    - Populate window with data from win->out_file_path
    - "Scroll" window data
    - new files must have window_t data calculated by update_window_data()

    - key:
        BEGINNING
        KEY_UP
        KEY_DOWN
        KEY_RIGHT
        KEY_LEFT

    TODO: page up/down, home/end
*/
static void 
render_data (window_t *win, 
             int key) 
{
    char  line[256];
    int   row = 1,
          col = 1,
          print_line,
          line_len,
          line_index,
          i, j;
    FILE *fp;

    // open output file
    fp = fopen (win->out_file_path, "r");
    if (fp == NULL) {
        pfeme ("Unable to open output file at \"%s\"", win->out_file_path);
    }

#ifndef DEBUG
    // clear window
    for (i = 1; i <= win->dwin_rows; i++) {
        for (j = 1; j <= win->dwin_cols; j++) {
            mvwaddch (win->win, i, j, ' ');
        }
    }
#endif

    // shift mid_line, first_char
    switch (key) {
        case BEGINNING:     // TODO: rename to FIRST_OPEN (?)
            break;
        case KEY_UP:
            win->win_mid_line = (win->win_mid_line <= win->file_min_mid) ? win->file_min_mid : win->win_mid_line - 1;
            break;
        case KEY_DOWN:
            win->win_mid_line = (win->win_mid_line >= win->file_max_mid) ? win->file_max_mid : win->win_mid_line + 1;
            break;
        case KEY_RIGHT:
            win->file_first_char = ((win->file_max_cols - win->file_first_char) > win->dwin_cols)
                ? win->file_first_char + 1
                : win->file_max_cols - win->dwin_cols;
            break;
        case KEY_LEFT:
            win->file_first_char = (win->file_first_char == 0) ? 0 : win->file_first_char - 1;
            break;
    }

    // get starting line
    print_line = win->win_mid_line - (win->dwin_rows / 2);

    // print lines to window
    for (i = 0; i < win->dwin_rows; i++) {

        // seek to beginning of line
        fseek (fp, win->file_offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), fp);
        line_len = strlen (line);

        // if line characters visible
        if (win->file_first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n')
                line_len -= 1;

            // calculate line length
            line_len = ((line_len - win->file_first_char) <= win->dwin_cols) 
                        ? line_len - win->file_first_char
                        : win->dwin_cols;

            // set line start index
            line_index = win->file_first_char;
        } 

        // if no characters visible
        else {
            line [0] = ' ';
            line_len = 1;
            line_index = 0;
        }

#ifndef DEBUG
        // print line to window
        mvwaddnstr (win->win, row++, col, (const char *)(line + line_index), line_len);
#endif

        // break if end of file
        if (print_line > win->file_rows) 
            break;
    }

#ifndef DEBUG
    wrefresh(win->win);
#endif

#ifdef DEBUG
    printf ("RENDER WINDOW DATA (%c)\n", win->key);
    puts   ("--------------");
    printf ("file_rows: %d\n", win->file_rows);
    printf ("file_max_cols: %d\n", win->file_max_cols);
    for (i = 1; i < win->file_rows; i++) {
        printf ("file_offsets [%d] : %ld\n", i, win->file_offsets [i]);
    }
    printf ("file_min_mid: %d\n", win->file_min_mid);
    printf ("file_max_mid: %d\n", win->file_max_mid);
    printf ("file_first_char: %d\n", win->file_first_char);
    puts   ("--------------");
    printf ("OUTPUT FILE DATA\n");
    puts   ("--------------");
    printf ("%s\n", win->out_file_path);
    puts   ("--------------");
    int ch;
    rewind (fp);
    while ((ch = fgetc (fp)) != EOF) {
        putchar (ch);
    }
    puts   ("--------------");
    puts   ("");
#endif

    fclose (fp);
}



static void 
pulse_header_title_color (state_t *state, 
                          int      pulse_state)
{
#ifndef DEBUG

    size_t i;
    int x, y,
        title_color,
        keych_color;
    bool key_color_toggle,
         string_exists;
    
    WINDOW *win   = state->layouts->header;
    char   *title = state->curr_plugin->title;

    // set title color variables
    switch (pulse_state) {
        case HEADER_TITLE_COLOR_ON:
            title_color = FOCUS_HEADER_TITLE_COLOR;
            keych_color = FOCUS_TITLE_KEY_COLOR;
            break;
        case HEADER_TITLE_COLOR_OFF:
            title_color = HEADER_TITLE_COLOR;
            keych_color = TITLE_KEY_COLOR;
    }

    // get title string location
    string_exists = find_window_string (win, title, &y, &x);

    // change title string color
    if (string_exists) {

        key_color_toggle = false;
        set_nc_attribute (win, title_color);
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (win, y, x + i, "%c", title [i]);
            if (key_color_toggle) {
                set_nc_attribute (win, title_color);
                key_color_toggle = false;
            }
            if (title [i] == '(') {
                set_nc_attribute (win, keych_color);
                key_color_toggle = true;
            }
        }
        wrefresh (win);
    }
#endif

#ifdef DEBUG
    (void) state;
    (void) pulse_state;
#endif
}



void select_window (state_t *state)
{
    size_t i;
    int x, y;
    bool key_color_toggle,
         string_exists;

    if (curr_win)
        deselect_window ();

    curr_win = state->curr_window;
    curr_title = state->curr_plugin->title;

    string_exists = find_window_string (curr_win->win, curr_title, &y, &x);

    curr_win->selected = true;

#ifndef DEBUG

    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->win, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR) | A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->win, y, x + i, "%c", curr_title [i]);

            if (key_color_toggle) {
                wattron (curr_win->win, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            if (curr_title [i] == '(') {
                wattron (curr_win->win, COLOR_PAIR(FOCUS_WINDOW_TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (curr_win->win);
    }

#endif

#ifdef DEBUG
    (void) key_color_toggle;
    (void) i;
#endif
}



void deselect_window (void)
{
    size_t i;
    int x, y;
    bool key_color_toggle,
         string_exists;

    string_exists = find_window_string (curr_win->win, curr_title, &y, &x);

    curr_win->selected = false;

#ifndef DEBUG

    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->win, COLOR_PAIR(WINDOW_TITLE_COLOR));
        wattroff (curr_win->win, A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->win, y, x + i, "%c", curr_title[i]);

            if (key_color_toggle) {
                wattron (curr_win->win, COLOR_PAIR(WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            if (curr_title[i] == '(') {
                wattron (curr_win->win, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (curr_win->win);
    }

#endif

    curr_win = NULL;
    curr_title [0] = '\0';

#ifdef DEBUG
    (void) key_color_toggle;
    (void) i;
#endif
}



/*
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates
*/
static bool find_window_string (WINDOW *window,
                                char   *string,
                                int    *y,
                                int    *x)
{
#ifndef DEBUG

    int  i, j, 
         m, n,
         ch,
         rows, cols;
    size_t si = 0;
    bool found = false;

    // get number of rows, columns
    getmaxyx (window, rows, cols);

    // find string
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            ch = mvwinch (window, i, j);

            if ((char) ch == string [si]) {
                if (si == 0) {
                    m = i;
                    n = j;
                }
                si += 1;
                if (si == strlen (string)) {
                    found = true;
                    break;
                }
            } else {
                si = 0;
            }
        }
        if (found) {
            break;
        }
    }

    if (found) {
        *y = m;
        *x = n;
        return true;
    } else {
        return false;
    }

#endif

#ifdef DEBUG

    (void) window;
    (void) string;
    (void) *y;
    (void) *x;

#endif

}

