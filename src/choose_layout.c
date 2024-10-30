#include <string.h>
#include <ncurses.h>

#include "data.h"
#include "choose_layout.h"
#include "render_layout.h"
#include "update_window_data/_update_window_data.h"
#include "plugins.h"
#include "utilities.h"

static int close_popup_window_selection (void);

#define ESC  27

WINDOW *curr_screen_layout_sel,
       *parent_popup_win_sel,
       *data_popup_win_sel;



int
choose_layout (state_t *state)
{
    int scr_rows, scr_cols,
        rows, cols,
        y, x,
        ch,
        num_options,
        curr_option,
        print_option,
        ret;
    layout_t *curr_layout;
    bool new_layout;

    // create layouts buffer, calculate max columns, rows
    rows = 0;
    cols = 0;
    num_options = 0;
    curr_layout = state->layouts;
        //
    do {
        cols = (cols > (int) strlen (curr_layout->label) + 2) ? cols : (int) strlen (curr_layout->label) + 2;
        ++rows;
        ++num_options;
        curr_layout = curr_layout->next;
    } while (curr_layout != NULL);
    rows += 3;
    cols += 3;

    keypad (stdscr, TRUE);

    // save current layout
    doupdate ();
    curr_screen_layout_sel = dupwin (curscr);

    // create popup window
    getmaxyx (stdscr, scr_rows, scr_cols);
    rows = (rows < scr_rows) ? rows : scr_rows;
    cols = (cols < scr_cols) ? cols : scr_cols;
    y = (scr_rows > rows) ? ((scr_rows - rows) / 2) : 1;
    x = (scr_cols > cols) ? ((scr_cols - cols) / 2) : 1;

        // parent
    parent_popup_win_sel = newwin (rows, cols, y, x);
    box (parent_popup_win_sel, 0, 0);
    wrefresh (parent_popup_win_sel);

        // data
    data_popup_win_sel = derwin (parent_popup_win_sel, rows - 2, cols - 2, 1, 1);

    // print options
    // TODO: highlight current layout on initial open
    new_layout = false;
    curr_layout = state->layouts;
    curr_option = 0;
    print_option = 0;
    do {
        if (strcmp (curr_layout->label, state->curr_layout->label) == 0) {
            wattron  (data_popup_win_sel, A_REVERSE);
            waddnstr (data_popup_win_sel, curr_layout->label, cols - 3);
            wattroff (data_popup_win_sel, A_REVERSE);
            curr_option = print_option;
        } else {
            waddnstr (data_popup_win_sel, curr_layout->label, cols - 3);
        }
        waddstr  (data_popup_win_sel, "\n");
        ++print_option;
        curr_layout = curr_layout->next;
    } while (curr_layout != NULL);
    wrefresh (data_popup_win_sel);

    // select option
    while ((ch = getch ()) != 'q' &&
                        ch != ESC) {

        // navigate up/down
        switch (ch) {

            case 'k':
            case KEY_UP:
                if (curr_option > 0) {
                    --curr_option;
                }
                break;

            case 'j':
            case KEY_DOWN:
                if (curr_option < num_options) {
                    ++curr_option;
                }
                break;

            case '\n':
                curr_layout = state->layouts;
                do {
                    if (curr_option-- == 0) {
                        state->curr_layout = curr_layout;
                        new_layout = true;
                        break;
                    }
                    curr_layout = curr_layout->next;
                } while (curr_layout != NULL);

                goto choose_layout;
        }

        // reprint options
        curr_layout = state->layouts;
        print_option = 0;
        werase (data_popup_win_sel);
            //
        do {
            if (print_option == curr_option) {
                wattron  (data_popup_win_sel, A_REVERSE);
                waddnstr (data_popup_win_sel, curr_layout->label, cols - 3);
                wattroff (data_popup_win_sel, A_REVERSE);
            } else {
                waddnstr (data_popup_win_sel, curr_layout->label, cols - 3);
            }
            waddstr  (data_popup_win_sel, "\n");
            ++print_option;
            curr_layout = curr_layout->next;
        } while (curr_layout != NULL);

        wrefresh (data_popup_win_sel);
    }

choose_layout:

    ret = close_popup_window_selection ();
    if (ret == FAIL) {
        pfemr ("Failed to close popup window for layout selection \"%s\"",
                    curr_layout->label);
    }

    keypad (stdscr, FALSE);

    if (new_layout) {

        ret = render_layout (curr_layout->label, state);
        if (ret == FAIL) {
            pfemr (ERR_REND_LAYOUT);
        }

        state->debugger->src_path_changed = true;
        ret = update_windows (state, 9, Dbg, Prg, Src, Asm, Brk, LcV, Reg, Stk, Wat);
        if (ret == FAIL) {
            pfemr (ERR_UPDATE_WINS);
        }
    }

    return A_OK;
}



static int
close_popup_window_selection (void)
{
    int ret;

    touchwin (curr_screen_layout_sel);
    ret = wnoutrefresh (curr_screen_layout_sel);
    if (ret == ERR) {
        pfemr ("wnoutrefresh failed");
    }
    doupdate ();
    curs_set (0);
    noecho ();

    ret = delwin (curr_screen_layout_sel);
    if (ret == ERR) {
        pfemr ("Failed to delete saved current screen");
    }
    ret = delwin (data_popup_win_sel);
    if (ret == ERR) {
        pfemr ("Failed to delete data popup window");
    }
    ret = delwin (parent_popup_win_sel);
    if (ret == ERR) {
        pfemr ("Failed to delete parent popup window");
    }

    return A_OK;
}



