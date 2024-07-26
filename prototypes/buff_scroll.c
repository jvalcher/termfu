
/*
   Subwindow, pad, scrolling
*/

#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

#define ESC         27
#define DBUFF_LEN   4096
#define IBUFF_LEN   256

typedef unsigned long int ulong;


typedef struct {


    char    *ptr;
    int      rows;
    int      max_cols;
    int      scroll_row;
    int      scroll_col;

} data_buff_t;


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

    data_buff_t *data_buff;

} window_t;


typedef struct {
    int line_num;
    char *path;
} breakpoint_t;


void get_buff_rows_cols (char *buffer, window_t *win);
void file_to_buffer (char *path, char *buffer);
void display_lines_buff (int, window_t*);



int main (void) 
{
    int ch;

    window_t *w = (window_t*) malloc (sizeof (window_t));
    w->data_buff = (data_buff_t*) malloc (sizeof (data_buff_t));

    unsigned long i;
    char data_buffer [DBUFF_LEN] = {0};

    // create data buffer
    file_to_buffer ("./src_file.c", data_buffer);   // mock buffer
    w->data_buff->ptr = data_buffer;
    w->data_buff->scroll_row = 1;
    w->data_buff->scroll_col = 1;
    get_buff_rows_cols (data_buffer, w);


    // create ncurses windows
    w->rows = 30;
    w->cols = 50;
    w->y = 20;
    w->x = 20;

    char *input_title = "(i)nput";
    char *input_prompt = "Input: ";
    w->input_title = input_title;
    w->input_prompt = input_prompt;
    w->input_rows = 1;
    w->input_cols = w->cols - 2;
    w->input_y = 1;
    w->input_x = 1;

    w->data_win_rows = w->rows - w->input_rows - 2;
    w->data_win_cols = w->cols - 2;
    w->data_win_y = w->input_y + 1;
    w->data_win_x = 1;

    initscr ();
    noecho ();
    keypad (stdscr, TRUE);
    curs_set (0);
    refresh ();

    start_color();
    init_pair (1, COLOR_WHITE, COLOR_BLUE);
    init_pair (2, COLOR_RED, COLOR_YELLOW);
    init_pair (3, COLOR_BLACK, COLOR_GREEN);

    // main win
    w->WIN = newwin (w->rows, w->cols, w->y, w->x);
    box (w->WIN, 0, 0);
    wrefresh (w->WIN);

    // input win
    w->IWIN = derwin (w->WIN, w->input_rows, w->input_cols, w->input_y, w->input_x);
    wbkgd (w->IWIN, COLOR_PAIR(2));
    waddstr (w->IWIN, w->input_title);

    // data win
    w->DWIN = derwin (w->WIN, w->data_win_rows, w->data_win_cols, w->data_win_y, w->data_win_x);

    wrefresh (w->WIN);

    // display lines
    display_lines_buff (0, w);
        //
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                display_lines_buff (KEY_UP, w);
                break;
            case KEY_DOWN:
                display_lines_buff (KEY_DOWN, w);
                break;
            case KEY_RIGHT:
                display_lines_buff (KEY_RIGHT, w);
                break;
            case KEY_LEFT:
                display_lines_buff (KEY_LEFT, w);
                break;
        }
    }

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin();

    return 0;
}



void
file_to_buffer (char *path,
                char *buffer)
{
    FILE *fp = fopen (path, "r");
    char *bp = buffer;
    while ((*bp++ = fgetc (fp)) != EOF) {
        ;
    }
    *bp = '\0';
    fclose (fp);
}



void
get_buff_rows_cols (char *buffer,
                    window_t *win)
{
    char *buff_ptr = buffer;
    int   curr_cols = 0;

    win->data_buff->rows = 1;
    win->data_buff->max_cols = 0;

    while (*buff_ptr != '\0') {
        if (*buff_ptr == '\n') {
            ++win->data_buff->rows;
            if (curr_cols > win->data_buff->max_cols) {
                win->data_buff->max_cols = curr_cols;
            }
            curr_cols = 0;
        } else {
            ++curr_cols;
        }
        ++buff_ptr;
    }
}



void
display_lines_buff (int key,
                    window_t *win)
{
    char   *buff_ptr = win->data_buff->ptr,
           *newline_ptr;
    int     wy = 0,
            wx = 0,
            data_row;

    wclear (win->DWIN);

    switch (key) {
        case 0:
            break;
        case KEY_UP:
            if (win->data_buff->scroll_row > 1) {
                --win->data_buff->scroll_row;
            }
            break;
        case KEY_DOWN:
            if ((win->data_buff->rows - win->data_buff->scroll_row) >= win->data_win_rows) {
                ++win->data_buff->scroll_row;
            }
            break;
        case KEY_LEFT:
            if (win->data_buff->scroll_col > 1) {
                --win->data_buff->scroll_col;
            }
            break;
        case KEY_RIGHT:
            if ((win->data_buff->max_cols - win->data_buff->scroll_col) >= win->data_win_cols) {
                ++win->data_buff->scroll_col;
            }
            break;
    }

    // move buffer pointer to beginning of current row
    data_row = 1;
    while (data_row != win->data_buff->scroll_row) {
        if (*buff_ptr++ == '\n') {
            ++data_row;
        }
    }

    // print buffer lines
    while ((newline_ptr = strchr(buff_ptr, '\n')) != NULL && wy < win->data_win_rows) {

        int len = newline_ptr - buff_ptr;

        if (len >= win->data_buff->scroll_col) {
            if (len - win->data_buff->scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - (win->data_buff->scroll_col - 1);
            }
            mvwprintw (win->DWIN, wy, wx, "%.*s", len, buff_ptr + (win->data_buff->scroll_col - 1));
        } else {
            mvwprintw (win->DWIN, wy, wx, " ");
        }

        buff_ptr = newline_ptr + 1;
        wx = 0;
        ++wy;
    }

    // print last line if needed
    if (*buff_ptr != '\0' && wy <= win->data_win_rows) {
        int len = strlen (buff_ptr);

        if (len > win->data_buff->scroll_col) {
            if (len - win->data_buff->scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - win->data_buff->scroll_col;
            }
            mvwprintw(win->DWIN, wy, wx, "%.*s", len, buff_ptr + win->data_buff->scroll_col);
        } else {
            mvwprintw(win->DWIN, wy, wx, " ");
        }
    }

    wrefresh (win->DWIN);
}
                
