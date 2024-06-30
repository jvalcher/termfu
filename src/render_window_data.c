
#include "render_window_data.h"
#include "data.h"
#include "utilities.h"

static void  render_data                (int, int, state_t*);
static void  pulse_header_title_color   (int, state_t*, int);
static void  select_window_color        (int, state_t*);
static void  deselect_window_color      (void);

#ifndef DEBUG
    static bool  find_window_string         (WINDOW*, char*, int*, int*);
#endif

char     *curr_title = NULL;
window_t *curr_win   = NULL;
WINDOW   *curr_WIN   = NULL;
 


/*
    Render window changes
*/
void 
render_window_data (int       plugin_index,
                    state_t  *state,
                    int       key,
                    int       type) 
{
    switch (type) {

        case HEADER_TITLE_COLOR_ON:
            pulse_header_title_color (plugin_index, state, HEADER_TITLE_COLOR_ON);
            break;

        case HEADER_TITLE_COLOR_OFF:
            pulse_header_title_color (plugin_index, state, HEADER_TITLE_COLOR_OFF);
            break;

        case SELECT_WINDOW_COLOR:
            select_window_color (plugin_index, state);
            break;

        case DESELECT_WINDOW_COLOR:
            deselect_window_color ();
            break;

        case RENDER_DATA:
            render_data (key, plugin_index, state);
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
render_data (int      key,
             int      plugin_index,
             state_t *state)
{
    (void) key;
    (void) plugin_index;
    (void) state;
    /*
    char      line [256];
    int       row = 1,
              col = 1,
              print_line = 0,
              line_len = 0,
              line_index = 0,
              i;
    window_t *win;

    win = state->windows[plugin_index];

    // open output file
    fp = fopen (win->out_file_path, "r");
    if (fp == NULL) {
        pfeme ("Unable to open output file at \"%s\"", win->out_file_path);
    }

#ifndef DEBUG
    int j;
    // clear window
    for (i = 1; i <= win->dwin_rows; i++) {
        for (j = 1; j <= win->dwin_cols; j++) {
            mvwaddch (win->WIN, i, j, ' ');
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
        mvwaddnstr (win->WIN, row++, col, (char*)(line + line_index), line_len);
#endif

        // break if end of file
        if (print_line > win->file_rows) 
            break;
    }

#ifndef DEBUG
    wrefresh(win->WIN);
#endif

#ifdef DEBUG
    printf ("RENDER WINDOW DATA (%c)\n", state->plugins[plugin_index]->key);
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

    (void) line_index;
    (void) col;
    (void) row;

    fclose (fp);
    */
}



static void 
pulse_header_title_color (int      plugin_index, 
                          state_t *state,
                          int      pulse_state)
{
#ifndef DEBUG

    size_t  i;
    int     x, y,
            title_color,
            keych_color;
    bool    key_color_toggle,
            string_exists;
    WINDOW *header;
    char   *title;

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

    header = state->header;
    title  = state->plugins[plugin_index]->title;
    
    // get title string location
    string_exists = find_window_string (header, title, &y, &x);

    // change title string color
    if (string_exists) {

        key_color_toggle = false;
        set_nc_attribute (header, title_color);
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (header, y, x + i, "%c", title [i]);
            if (key_color_toggle) {
                set_nc_attribute (header, title_color);
                key_color_toggle = false;
            }
            if (title [i] == '(') {
                set_nc_attribute (header, keych_color);
                key_color_toggle = true;
            }
        }
        wrefresh (header);
    }
#endif

#ifdef DEBUG
    (void) plugin_index;
    (void) state;
    (void) pulse_state;
#endif
}



static void
select_window_color (int      plugin_index,
                     state_t *state)
{
    if (curr_win)
        deselect_window_color ();

    curr_win = state->windows[plugin_index];
    curr_title = state->plugins[plugin_index]->title;

    curr_win->selected = true;


#ifndef DEBUG

    size_t i;
    int    x, y;
    bool   key_color_toggle,
           string_exists;

    string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x);

    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR) | A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->WIN, y, x + i, "%c", curr_title [i]);

            if (key_color_toggle) {
                wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            if (curr_title [i] == '(') {
                wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (curr_win->WIN);
    }

#endif
}



void
deselect_window_color (void)
{

    curr_win->selected = false;

#ifndef DEBUG

    size_t i;
    int x, y;
    bool key_color_toggle,
         string_exists;

    string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x);

    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->WIN, COLOR_PAIR(WINDOW_TITLE_COLOR));
        wattroff (curr_win->WIN, A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->WIN, y, x + i, "%c", curr_title[i]);

            if (key_color_toggle) {
                wattron (curr_win->WIN, COLOR_PAIR(WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            if (curr_title[i] == '(') {
                wattron (curr_win->WIN, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (curr_win->WIN);
    }

#endif

    curr_win = NULL;
    curr_title [0] = '\0';
}


#ifndef DEBUG

/*
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates
*/
static bool
find_window_string (WINDOW *window,
                    char   *string,
                    int    *y,
                    int    *x)
{

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
}

#endif
