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
    bool               path_changed;
    int                first_char;
    int                rows;
    int                max_cols;
    int                min_mid;
    int                max_mid;
    unsigned long int *offsets;

} file_data_t;


typedef struct {

    WINDOW  *WIN;
    WINDOW  *IWIN;
    WINDOW  *DWIN;

    bool     selected;

    int      rows;                   
    int      cols;                   
    int      y;                      
    int      x;                      
    int      border [8];

    int      input_rows;
    int      input_cols;
    int      input_y;
    int      input_x;
    char    *input_title;
    char    *input_prompt;
    bool     has_input;
    char    *input_inactive_str;
    char    *input_active_str;

    int      data_win_cols;
    int      data_win_rows;
    int      data_win_y;
    int      data_win_x;
    int      data_win_mid_line;

    file_data_t *file_data;

} window_t;

void get_num_file_rows_cols (window_t* win);
void display_lines_file (int key, window_t* win);



int main (void) 
{
    int     i, ch;

    window_t *win = (window_t*) malloc (sizeof (window_t));
    win->file_data = (file_data_t*) malloc (sizeof (file_data_t));

    // open source file
    strncpy (win->file_data->path, "./src_file.c", sizeof (win->file_data->path) - 1);
    win->file_data->ptr = fopen(win->file_data->path, "r");
    if (win->file_data->ptr == NULL) {
        perror ("Failed to open source file");
        return 1;
    }
    win->file_data->path_changed = false;

        // get number of file rows and max line length
    get_num_file_rows_cols (win);

        // get file line offsets
    win->file_data->offsets = malloc ((size_t) win->file_data->rows * sizeof(long int));
    if (win->file_data->offsets == NULL) {
        perror ("Failed to allocate offsets array");
        return 1;
    }
        // add offsets to array
    win->file_data->offsets [0] = 0;
    for (i = 1; i < win->file_data->rows; i++) {
        while ((ch = fgetc (win->file_data->ptr)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            win->file_data->offsets[i] = ftell (win->file_data->ptr);
    }

    // create ncurses windows
    win->rows = 30;
    win->cols = 50;
    win->y = 20;
    win->x = 20;

    char *input_title = "(i)nput";
    char *input_prompt = "Input: ";
    win->input_title = input_title;
    win->input_prompt = input_prompt;
    win->input_rows = 1;
    win->input_cols = win->cols - 2;
    win->input_y = 1;
    win->input_x = 1;

    win->data_win_rows = win->rows - win->input_rows - 2;
    win->data_win_cols = win->cols - 2;
    win->data_win_y = win->input_y + 1;
    win->data_win_x = 1;

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
    win->WIN = newwin (win->rows, win->cols, win->y, win->x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);

    // input win
    win->IWIN = derwin (win->WIN, win->input_rows, win->input_cols, win->input_y, win->input_x);
    wbkgd (win->IWIN, COLOR_PAIR(2));
    waddstr (win->IWIN, win->input_title);

    // data win
    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, win->data_win_y, win->data_win_x);

    wrefresh (win->WIN);

    // calculate file data
    win->file_data->min_mid = (win->data_win_rows / 2) + 1;
    win->file_data->max_mid = win->file_data->rows - ((win->data_win_rows - 1) / 2);
    win->data_win_mid_line = win->file_data->min_mid;
    win->file_data->first_char = 0;

    // display starting lines
    display_lines_file(0, win);
        //
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                display_lines_file (KEY_UP, win);
                break;
            case KEY_DOWN:
                display_lines_file (KEY_DOWN, win);
                break;
            case KEY_RIGHT:
                display_lines_file (KEY_RIGHT, win);
                break;
            case KEY_LEFT:
                display_lines_file (KEY_LEFT, win);
                break;
        }
    }

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin ();

    return 0;
}



void get_num_file_rows_cols (window_t *win) {
    win->file_data->rows = 0;
    win->file_data->max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), win->file_data->ptr) != NULL) {
        line_len = strlen(line);
        if (win->file_data->max_cols < line_len) {
            win->file_data->max_cols = line_len + 1;
        }
        win->file_data->rows += 1;
    }
    rewind (win->file_data->ptr);
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
            win->data_win_mid_line = (win->data_win_mid_line <= win->file_data->min_mid) ? win->file_data->min_mid : win->data_win_mid_line - 1;
            break;
        case KEY_DOWN:
            win->data_win_mid_line = (win->data_win_mid_line >= win->file_data->max_mid) ? win->file_data->max_mid : win->data_win_mid_line + 1;
            break;
        case KEY_RIGHT:
            win->file_data->first_char = ((win->file_data->max_cols - win->file_data->first_char) > win->data_win_cols)
                ? win->file_data->first_char + 1
                : win->file_data->max_cols - win->data_win_cols;
            break;
        case KEY_LEFT:
            win->file_data->first_char = (win->file_data->first_char == 0) ? 0 : win->file_data->first_char - 1;
            break;
    }

    // calculate first line
    print_line = win->data_win_mid_line - (win->data_win_rows / 2);

    // print lines
    for (i = 0; i < win->data_win_rows; i++) {

        // seek to beginning of line
        fseek (win->file_data->ptr, win->file_data->offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), win->file_data->ptr);
        line_len = strlen (line);

        // if line characters visible
        if (win->file_data->first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n')
                line_len -= 1;

            // calculate line length
            line_len = ((line_len - win->file_data->first_char) <= win->data_win_cols) 
                        ? line_len - win->file_data->first_char
                        : win->data_win_cols;

            // set line start index
            line_index = win->file_data->first_char;
        } 

        // if no characters visible
        else {
            line [0] = ' ';
            line_len = 1;
            line_index = 0;
        }

        mvwaddnstr (win->DWIN, row++, col, (const char*)(line + line_index), line_len);

        // break if end of file
        if (print_line > win->file_data->rows) {
            break;
        }
    }

    wrefresh(win->DWIN);
}
