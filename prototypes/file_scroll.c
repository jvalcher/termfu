/*
    Ncurses pad tests
*/
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

#define DBUFF_LEN   4096

typedef struct {

    FILE              *ptr;
    char               path [256];
    int                first_char;
    int                rows;
    int                max_cols;
    int                min_mid;
    int                max_mid;
    unsigned long int *offsets;

} file_scroll_t;


typedef struct {

    WINDOW  *WIN;
    WINDOW  *IWIN;
    WINDOW  *DWIN;
    bool     selected;

    bool     has_input;
    char    *input_inactive_str;
    char    *input_active_str;

    int      rows;                   
    int      cols;                   
    int      y;                      
    int      x;                      
    int      border [8];

    char    *data_buff_ptr;
    int      data_buff_rows;
    int      data_buff_max_cols;
    int      data_win_cols;
    int      data_win_rows;
    int      data_scroll_row;
    int      data_scroll_col;
    int      data_win_mid_line;

    file_scroll_t *file_scroll;

} window_t;

void get_num_file_rows_cols (window_t* win);
void display_lines_file (int key, window_t* win);



int main (void) 
{
    int     i, ch;
    WINDOW *win;
    WINDOW *input_win;
    WINDOW *data_win;

    window_t *window = (window_t*) malloc (sizeof (window_t));
    file_scroll_t *fs = (file_scroll_t*) malloc (sizeof (file_scroll_t));
    window->file_scroll = fs;

    // open source file
    strncpy (window->file_scroll->path, "./src_file.c", sizeof (window->file_scroll->path) - 1);
    window->file_scroll->ptr = fopen(window->file_scroll->path, "r");
    if (window->file_scroll->ptr == NULL) {
        perror ("Failed to open source file");
        return 1;
    }

        // get number of file rows and max line length
    get_num_file_rows_cols (window);

        // get file line offsets
    window->file_scroll->offsets = malloc ((size_t) window->file_scroll->rows * sizeof(long int));
    if (window->file_scroll->offsets == NULL) {
        perror ("Failed to allocate offsets array");
        return 1;
    }
        // add offsets to array
    window->file_scroll->offsets [0] = 0;
    for (i = 1; i < window->file_scroll->rows; i++) {
        while ((ch = fgetc (window->file_scroll->ptr)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            window->file_scroll->offsets [i] = ftell (window->file_scroll->ptr);
    }

    // create ncurses windows
    int lines = 30;
    int cols = 50;
    int y = 20;
    int x = 20;

    char *input_title = "(i)nput";
    int input_lines = 1;
    int input_cols = cols - 2;
    int input_y = 1;
    int input_x = 1;

    int data_lines = lines - input_lines - 2;
    int data_cols = cols - 2;
    int data_y = input_y + 1;
    int data_x = 1;

    window->rows = lines;
    window->cols = cols;
    window->y = 20;
    window->x = 20;
    window->data_win_rows = data_lines;
    window->data_win_cols = data_cols;

        // Initialize ncurses
    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
    refresh ();

    start_color();
    init_pair (1, COLOR_WHITE, COLOR_BLUE);
    init_pair (2, COLOR_RED, COLOR_YELLOW);
    init_pair (3, COLOR_BLACK, COLOR_GREEN);

    // main win
    win = newwin (lines, cols, y, x);
    box (win, 0, 0);
    wrefresh (win);

    // input win
    input_win = derwin (win, input_lines, input_cols, input_y, input_x);
    wbkgd (input_win, COLOR_PAIR(2));
    waddstr (input_win, input_title);

    // data win
    data_win = derwin (win, data_lines, data_cols, data_y, data_x);

    wrefresh (win);

    window->WIN = win;
    window->IWIN = input_win;
    window->DWIN = data_win;

    window->file_scroll->min_mid = (window->data_win_rows / 2) + 1;
    window->file_scroll->max_mid = window->file_scroll->rows - ((window->data_win_rows - 1) / 2);
    window->data_win_mid_line = window->file_scroll->min_mid;
    window->file_scroll->first_char = 0;

    // display starting lines
    display_lines_file(0, window);
        //
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                display_lines_file (KEY_UP, window);
                break;
            case KEY_DOWN:
                display_lines_file (KEY_DOWN, window);
                break;
            case KEY_RIGHT:
                display_lines_file (KEY_RIGHT, window);
                break;
            case KEY_LEFT:
                display_lines_file (KEY_LEFT, window);
                break;
        }
    }

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin ();

    return 0;
}



void get_num_file_rows_cols (window_t *win) {
    win->file_scroll->rows = 0;
    win->file_scroll->max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), win->file_scroll->ptr) != NULL) {
        line_len = strlen(line);
        if (win->file_scroll->max_cols < line_len) {
            win->file_scroll->max_cols = line_len + 1;
        }
        win->file_scroll->rows += 1;
    }
    rewind (win->file_scroll->ptr);
}



/*
    Display file lines shifted according to key, i.e.
*/
void display_lines_file (int key,
                         window_t *win) 
{
    char line[256];
    int  row = 0,
         col = 0,
         print_line,
         line_len,
         line_index,
         i;

    wclear (win->DWIN);

    // shift mid_line, first_char
    switch (key) {
        case 0:
            break;
        case KEY_UP:
            win->data_win_mid_line = (win->data_win_mid_line <= win->file_scroll->min_mid) ? win->file_scroll->min_mid : win->data_win_mid_line - 1;
            break;
        case KEY_DOWN:
            win->data_win_mid_line = (win->data_win_mid_line >= win->file_scroll->max_mid) ? win->file_scroll->max_mid : win->data_win_mid_line + 1;
            break;
        case KEY_RIGHT:
            win->file_scroll->first_char = ((win->file_scroll->max_cols - win->file_scroll->first_char) > win->data_win_cols)
                ? win->file_scroll->first_char + 1
                : win->file_scroll->max_cols - win->data_win_cols;
            break;
        case KEY_LEFT:
            win->file_scroll->first_char = (win->file_scroll->first_char == 0) ? 0 : win->file_scroll->first_char - 1;
            break;
    }

    // calculate first line
    print_line = win->data_win_mid_line - (win->data_win_rows / 2);

    // print lines
    for (i = 0; i < win->data_win_rows; i++) {

        // seek to beginning of line
        fseek (win->file_scroll->ptr, win->file_scroll->offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), win->file_scroll->ptr);
        line_len = strlen (line);

        // if line characters visible
        if (win->file_scroll->first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n')
                line_len -= 1;

            // calculate line length
            line_len = ((line_len - win->file_scroll->first_char) <= win->data_win_cols) 
                        ? line_len - win->file_scroll->first_char
                        : win->data_win_cols;

            // set line start index
            line_index = win->file_scroll->first_char;
        } 

        // if no characters visible
        else {
            line [0] = ' ';
            line_len = 1;
            line_index = 0;
        }

        mvwaddnstr (win->DWIN, row++, col, (const char*)(line + line_index), line_len);

        // break if end of file
        if (print_line > win->file_scroll->rows) {
            break;
        }
    }

    wrefresh(win->DWIN);
}
