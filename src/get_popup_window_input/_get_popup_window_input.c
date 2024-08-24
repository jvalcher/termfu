#include <ncurses.h>

#include "_get_popup_window_input.h"


void close_popup_window (void);


WINDOW *curr_screen_layout,
       *parent_popup_win,
       *data_popup_win;



void
get_popup_window_input (char *prompt,
                        char *buffer)
{
    int scr_rows, scr_cols,
        rows, cols,
        y, x;

    curs_set (1);
    echo ();

    // save current layout
    doupdate ();
    curr_screen_layout = dupwin (curscr);

    // create popup window
    getmaxyx (stdscr, scr_rows, scr_cols);
    rows = 6;
    cols = (scr_cols > 60) ? 60 : (scr_cols - 2);
    y = (scr_rows > rows) ? ((scr_rows - rows) / 2) : 1;
    x = (scr_cols > cols) ? ((scr_cols - cols) / 2) : 1;

        // parent
    parent_popup_win = newwin (rows, cols, y, x);
    box (parent_popup_win, 0, 0);
    wrefresh (parent_popup_win);

        // data
    data_popup_win = derwin (parent_popup_win, rows - 2, cols - 2, 1, 1);
    mvwprintw (data_popup_win, 0, 0, "%s", prompt);
    wrefresh (data_popup_win);

    // get input
    // TODO: get each character individually to enable ESC, arrow keys, etc.
    wgetnstr (data_popup_win, buffer, sizeof (buffer) - 1);

    close_popup_window ();
}



void
close_popup_window (void)
{
    touchwin (curr_screen_layout);
    wnoutrefresh (curr_screen_layout);
    doupdate ();
    curs_set (0);
    noecho ();

    delwin (curr_screen_layout);
    delwin (data_popup_win);
    delwin (parent_popup_win);
}



