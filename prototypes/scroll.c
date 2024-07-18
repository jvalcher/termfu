
/*
   Subwindow, pad, scrolling
*/

#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

#define BUFF_LEN  256

char *src_lines = ""
"static void"
"allocate_windows (state_t *state)"
"{"
"    state->windows = (window_t**) malloc (state->num_plugins * sizeof (window_t*));"
"    if (state->windows == NULL) {"
"        pfeme (\"window_t pointer array allocation failed\");"
"    }"
"    for (int i = 0; i < state->num_plugins; i++) {"
"        state->windows [i] = (window_t*) malloc (sizeof (window_t));"
"        if (state->windows [i] == NULL) {"
"            pfeme (\"window_t pointer allocation failed\");"
"        }"
"    }"
"}"
""
""
""
"static void"
"free_window_data (state_t *state)"
"{"
"    for (int i = 0; i < state->num_plugins; i++) {"
"        if (state->windows [i]) {"
"            delwin (state->windows[i]->WIN);"
"        }"
"    }"
"}";

typedef unsigned long int ulong;

typedef struct {
    int      buff_max_cols;
    int      buff_rows;
} window_t;

// allocate string from buffer
char*
allocate_buffer (char *buffer, window_t *win)
{
    int rows = 1,
        max_cols = 0,
        cols = 0;
    char *buff_ptr;

    // calculate lines, max_cols
    buff_ptr = buffer;
    do {
        if (*buff_ptr == '\n') {
            rows += 1;
            ++cols;
            if (cols > max_cols) {
                max_cols = cols;
            }
        } else {
            ++cols;
        }
    } while (*buff_ptr++ != '\0');
    if (cols > max_cols) {
        max_cols = cols;
    }
    win->buff_rows = rows;
    win->buff_max_cols = max_cols;

    // allocate
    char *str = (char*) malloc (strlen (buffer) + 1);
    if (str == NULL) {
        fprintf (stderr, "Buffer allocation failed");
    }
    strncpy (str, buffer, strlen(buffer) + 1);
    return str;
}

// insert line numbers

// jump to line number with input field

// line highlight, select line for breakpoint


// breakpoints
typedef struct {
    int line_num;
    char *path;
} breakpoint_t;
    // create breakpoint
void
create_breakpoint (void) {}

// watchpoints


int main (void) 
{
    WINDOW *win;
    WINDOW *input_win;
    WINDOW *data_win;
r   
    unsigned long i;
    char input_buffer [BUFF_LEN];
    int ESC = 27;

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

    initscr ();
    refresh();
    
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

    wrefresh (data_win);

    // 




    getch();

    endwin();
}
