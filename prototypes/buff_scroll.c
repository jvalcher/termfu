
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

} window_t;


typedef struct {
    int line_num;
    char *path;
} breakpoint_t;


void get_buff_rows_cols (char *buffer, window_t *win);
void file_to_buffer (char *path, char *buffer);
void display_lines (int, window_t*);



int main (void) 
{
    int ch;
    WINDOW *win;
    WINDOW *input_win;
    WINDOW *data_win;

    window_t *window = (window_t*) malloc (sizeof (window_t));

    unsigned long i;
    char data_buffer [DBUFF_LEN] = {0};
    char input_buffer [IBUFF_LEN];

    // create data buffer
    file_to_buffer ("./src_file.c", data_buffer);   // mock buffer
    window->data_buff_ptr = data_buffer;
    window->data_scroll_row = 1;
    window->data_scroll_col = 1;
    get_buff_rows_cols (data_buffer, window);


    // create ncurses windows
    int lines = 30;
    int cols = 50;
    int y = 20;
    int x = 20;

    char *input_title = "(i)nput";
    char *input_str = "Input: ";
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

    // display lines
    display_lines (0, window);
        //
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case KEY_UP:
                display_lines (KEY_UP, window);
                break;
            case KEY_DOWN:
                display_lines (KEY_DOWN, window);
                break;
            case KEY_RIGHT:
                display_lines (KEY_RIGHT, window);
                break;
            case KEY_LEFT:
                display_lines (KEY_LEFT, window);
                break;
        }
    }

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

    win->data_buff_rows = 1;
    win->data_buff_max_cols = 0;

    while (*buff_ptr != '\0') {
        if (*buff_ptr == '\n') {
            ++win->data_buff_rows;
            if (curr_cols > win->data_buff_max_cols) {
                win->data_buff_max_cols = curr_cols;
            }
            curr_cols = 0;
        } else {
            ++curr_cols;
        }
        ++buff_ptr;
    }
}



void
display_lines (int key,
               window_t *win)
{
    char   *buff_ptr = win->data_buff_ptr,
           *newline_ptr;
    int     wy = 0,
            wx = 0,
            data_row;

    wclear (win->DWIN);

    switch (key) {
        case 0:
            break;
        case KEY_UP:
            if (win->data_scroll_row > 1) {
                --win->data_scroll_row;
            }
            break;
        case KEY_DOWN:
            if ((win->data_buff_rows - win->data_scroll_row) >= win->data_win_rows) {
                ++win->data_scroll_row;
            }
            break;
        case KEY_LEFT:
            if (win->data_scroll_col > 1) {
                --win->data_scroll_col;
            }
            break;
        case KEY_RIGHT:
            if ((win->data_buff_max_cols - win->data_scroll_col) >= win->data_win_cols) {
                ++win->data_scroll_col;
            }
            break;
    }

    // move buffer pointer to beginning of current row
    data_row = 1;
    while (data_row != win->data_scroll_row) {
        if (*buff_ptr++ == '\n') {
            ++data_row;
        }
    }

    // print buffer lines
    while ((newline_ptr = strchr(buff_ptr, '\n')) != NULL && wy < win->data_win_rows) {

        int len = newline_ptr - buff_ptr;

        if (len >= win->data_scroll_col) {
            if (len - win->data_scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - (win->data_scroll_col - 1);
            }
            mvwprintw (win->DWIN, wy, wx, "%.*s", len, buff_ptr + (win->data_scroll_col - 1));
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

        if (len > win->data_scroll_col) {
            if (len - win->data_scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - win->data_scroll_col;
            }
            mvwprintw(win->DWIN, wy, wx, "%.*s", len, buff_ptr + win->data_scroll_col);
        } else {
            mvwprintw(win->DWIN, wy, wx, " ");
        }
    }

    wrefresh (win->DWIN);
}
                
