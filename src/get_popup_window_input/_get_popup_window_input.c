#include <ncurses.h>

#include "_get_popup_window_input.h"
#include "../utilities.h"


int close_popup_window (void);

#define POPUP_ROWS  6

WINDOW *curr_screen_layout,
       *parent_popup_win,
       *data_popup_win;



int
get_popup_window_input (char *prompt,
                        char *buffer)
{
    int scr_rows, scr_cols,
        rows, cols,
        y, x,
        ret;

    curs_set (1);
    echo ();

    // save current layout
    doupdate ();
    curr_screen_layout = dupwin (curscr);
    if (curr_screen_layout == NULL) {
        pfem ("Failed to duplicate current screen");
        goto get_pop_in_err;
    }

    // create popup window
    getmaxyx (stdscr, scr_rows, scr_cols);
    rows = POPUP_ROWS;
    cols = (scr_cols > 60) ? 60 : (scr_cols - 2);
    y = (scr_rows > rows) ? ((scr_rows - rows) / 2) : 1;
    x = (scr_cols > cols) ? ((scr_cols - cols) / 2) : 1;

        // parent
    parent_popup_win = newwin (rows, cols, y, x);
    if (parent_popup_win == NULL) {
        pfem ("Failed to create parent window (%d, %d, %d, %d)", rows, cols, y, x);
        goto get_pop_in_err;
    }
    box (parent_popup_win, 0, 0);
    wrefresh (parent_popup_win);

        // data
    data_popup_win = derwin (parent_popup_win, rows - 2, cols - 2, 1, 1);
    if (data_popup_win == NULL) {
        pfem ("Failed to create data subwindow (%d, %d, %d, %d)", rows-2, cols-2, 1, 1);
        goto get_pop_in_err;
    }
    mvwprintw (data_popup_win, 0, 0, "%s", prompt);
    wrefresh (data_popup_win);

    // get input
    // TODO: get each character individually to enable escape, arrows, home, end, ...

    wgetnstr (data_popup_win, buffer, INPUT_BUFF_LEN - 1);

    ret = close_popup_window ();
    if (ret == FAIL) {
        pfem ("Failed to close popup window");
        goto get_pop_in_err;
    }

    return A_OK;

get_pop_in_err:

    pem  ("Prompt: %s", prompt);
    pemr ("Buffer: %s", buffer);
}



int
close_popup_window (void)
{
    int ret;

    touchwin (curr_screen_layout);
    wnoutrefresh (curr_screen_layout);
    doupdate ();
    curs_set (0);
    noecho ();

    ret = delwin (curr_screen_layout);
    if (ret == ERR) {
        pfemr ("(ERR)  delwin (curr_screen_layout)");
    }
    ret = delwin (data_popup_win);
    if (ret == ERR) {
        pfemr ("(ERR)  delwin (data_popup_win)");
    }
    ret = delwin (parent_popup_win);
    if (ret == ERR) {
        pfemr ("(ERR)  delwin (parent_popup_win)");
    }

    return A_OK;
}



