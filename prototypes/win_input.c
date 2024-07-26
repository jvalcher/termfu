
#include <ncurses.h>

#define BUFF_LEN  256


int
main (void)
{
    WINDOW *win;
    WINDOW *input_win;
    WINDOW *data_win;

    int lines = 30;
    int cols = 50;
    int y = 10;
    int x = 10;

    char *input_title = "(i)nput";
    char *input_str = "Input: ";
    int input_lines = 1;
    int input_cols = cols - 2;
    int input_y = 1;
    int input_x = 1;

    int data_lines = lines - input_lines - 2;
    int data_cols = cols - 2;
    int data_y = input_y + input_lines;
    int data_x = 1;

    initscr ();
    curs_set (0);
    refresh ();
    
    start_color ();
    init_pair (1, COLOR_WHITE, COLOR_BLUE);
    init_pair (2, COLOR_RED, COLOR_YELLOW);
    init_pair (3, COLOR_BLACK, COLOR_GREEN);

    // main win
    win = newwin (lines, cols, y, x);
    box (win, 0, 0);
    wrefresh (win);

    // input win
    input_win = derwin (win, input_lines, input_cols, input_y, input_x);
    wbkgd   (input_win, COLOR_PAIR(2));
    waddstr (input_win, input_title);

    data_win = derwin (win, data_lines, data_cols, data_y, data_x);

    wrefresh (win);

    // select input
    noecho ();
    getch ();
    curs_set (1);
    echo ();

    // get input
    mvwprintw (input_win, 0, 0, "%s", input_str);
    wrefresh  (input_win);
    wgetnstr  (input_win, input_buffer, BUFF_LEN - 1);

    // print input, return to title
    wclrtoeol (input_win);
    mvwprintw (input_win, 0, 0, "%s", input_title);
    curs_set (0);
    wrefresh  (input_win);

    // print input to data window
    mvwprintw (data_win, 2, 1, "%s", input_buffer);
    wrefresh (data_win);

    getch();

    endwin();
}
