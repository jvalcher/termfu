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
        y, x;

    curs_set (1);
    echo ();

    // save current layout
    doupdate ();
    if ((curr_screen_layout = dupwin (curscr)) == NULL) {
        pfem ("Failed to duplicate screen");
        goto get_pop_in_err;
    }

    // create popup window
    getmaxyx (stdscr, scr_rows, scr_cols);
    rows = POPUP_ROWS;
    cols = (scr_cols > 60) ? 60 : (scr_cols - 2);
    y = (scr_rows > rows) ? ((scr_rows - rows) / 2) : 1;
    x = (scr_cols > cols) ? ((scr_cols - cols) / 2) : 1;

        // parent
    if ((parent_popup_win = newwin (rows, cols, y, x)) == NULL) {
        pfem ("Failed to create parent input popup window");
        goto get_pop_in_err;
    }
    box (parent_popup_win, 0, 0);
    wrefresh (parent_popup_win);

        // data
    if ((data_popup_win = derwin (parent_popup_win, rows - 2, cols - 2, 1, 1)) == NULL) {
        pfem ("Failed to create data input popup window");
        goto get_pop_in_err;
    }
    mvwprintw (data_popup_win, 0, 0, "%s", prompt);
    wrefresh (data_popup_win);

    // get input
    // TODO: get each character individually to enable escape, arrows, home, end, ...

    wgetnstr (data_popup_win, buffer, INPUT_BUFF_LEN - 1);

    if (close_popup_window () == RET_FAIL) {
        pfem ("Failed to close input popup window");
        goto get_pop_in_err;
    }

    return RET_OK;

get_pop_in_err:

    pem  ("Prompt: %s", prompt);
    pemr ("Buffer: %s", buffer);
}



int
close_popup_window (void)
{
    touchwin (curr_screen_layout);
    if (wnoutrefresh (curr_screen_layout) == ERR) {
        pfemr ("wnoutrefresh failed");
    }
    doupdate ();
    curs_set (0);
    noecho ();

    if (delwin (curr_screen_layout) == ERR) {
        pfemr ("Failed to delete saved current screen");
    }
    if (delwin (data_popup_win) == ERR) {
        pfemr ("Failed to delete data popup window");
    }
    if (delwin (parent_popup_win) == ERR) {
        pfemr ("Failed to delete parent popup window");
    }

    return RET_OK;
}



