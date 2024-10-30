
#include "select_window.h"
#include "data.h"
#include "display_lines.h"
#include "get_form_input/watchpoints.h"
#include "get_form_input/breakpoints.h"
#include "utilities.h"
#include "plugins.h"

#define BUFF_LEN  256

static int  select_window_color    (int plugin_index, state_t *state);
static int  deselect_window_color  (void);

char     *curr_title = NULL;
window_t *curr_win   = NULL;
WINDOW   *curr_WIN   = NULL;



int
select_window (int      plugin_index,
               state_t *state)
{
    int           key,
                  ret;
    bool          in_loop         = true,
                  key_not_pressed = true;

    ret = select_window_color (plugin_index, state);
    if (ret == FAIL) {
        pfem ("Failed to select window color");
        goto sel_win_err;
    }

    keypad (stdscr, TRUE);

    while (in_loop) {

        key = getch ();

        switch (key) {

            case 'k':
            case KEY_UP:
            case 'j':
            case KEY_DOWN:
            case 'l':
            case KEY_RIGHT:
            case 'h':
            case KEY_LEFT:
            case KEY_PPAGE:
            case KEY_NPAGE:
            case KEY_END:
            case KEY_HOME:
                ret = display_lines (key, plugin_index, state);
                if (ret == FAIL) {
                    pfem ("Failed to display lines (%d)", key);
                    goto sel_win_err;
                }
                key_not_pressed = false;
                break;

            case 'q':
            case ESC:
                in_loop = false;
                key_not_pressed = false;
                break;
        }

        // custom keys
        if (in_loop && key_not_pressed) {

            // deselect window
            if (key == state->plugins[plugin_index]->key) {
                in_loop = false;
                continue;
            }

            // quit
            else if (key == state->plugins[Qut]->key) {
                in_loop = false;
                state->debugger->running = false;
                continue;
            }

            // specific window commands
            switch (plugin_index) {

                case Brk:
                    switch (key) {
                        case 'd': 
                            ret = delete_breakpoint (state);
                            if (ret == FAIL) {
                                pfem ("Failed to delete breakpoint");
                                goto sel_win_err;
                            }
                            in_loop = false;
                            break;
                        case 'c':
                            ret = insert_breakpoint (state);
                            if (ret == FAIL) {
                                pfem ("Failed to insert breakpoint");
                                goto sel_win_err;
                            }
                            in_loop = false;
                            break;
                        case 'a':
                            ret = clear_all_breakpoints (state);
                            if (ret == FAIL) {
                                pfem ("Failed to clear all breakpoints");
                                goto sel_win_err;
                            }
                            in_loop = false;
                            break;
                    }
                    break;

                case Wat:
                    switch (key) {
                        case 'd':
                            ret = delete_watchpoint (state);
                            if (ret == FAIL) {
                                pfem ("Failed to delete watchpoint");
                                goto sel_win_err;
                            }
                            in_loop = false;
                            break;
                        case 'c':
                            ret = insert_watchpoint (state);
                            if (ret == FAIL) {
                                pfem ("Failed to insert watchpoint");
                                goto sel_win_err;
                            }
                            in_loop = false;
                            break;
                        case 'a':
                            ret = clear_all_watchpoints (state);
                            if (ret == FAIL) {
                                pfem ("Failed to clear all watchpoints");
                                goto sel_win_err;
                            }
                            in_loop = false;
                            break;
                        }
                    }
                    break;
        }
        key_not_pressed = true;
    }

    ret = deselect_window_color ();
    if (ret == FAIL) {
        pfem ("Failed to deselect window color");
        goto sel_win_err;
    }

    keypad (stdscr, FALSE);

    return A_OK;

sel_win_err:
    pemr ("Select window error (index: %d, code: \"%s\")",
            plugin_index, get_plugin_code (plugin_index));
}



/*
    Change Ncurses window title color to indicate focus
*/
static int
select_window_color (int      plugin_index,
                     state_t *state)
{
    if (curr_win)
        deselect_window_color ();

    curr_win = state->plugins[plugin_index]->win;
    curr_title = state->plugins[plugin_index]->title;

    size_t i;
    int    x, y;
    bool   key_color_toggle,
           string_exists;

    if ((string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x)) == false) {
        pfemr ("Failed to find window title \"%s\"", curr_title);
    }


    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR) | A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->WIN, y, x + i, "%c", curr_title [i]);

            if (key_color_toggle) {
                wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            if (curr_title [i] == '(') {
                wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wattrset (curr_win->WIN, A_NORMAL);
        wrefresh  (curr_win->WIN);
    }

    return A_OK;
}



static int
deselect_window_color (void)
{

    size_t i;
    int x, y;
    bool key_color_toggle,
         string_exists;

    if ((string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x)) == false) {
        pfemr ("Failed to find window title \"%s\"", curr_title);
    }

    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->WIN, COLOR_PAIR(WINDOW_TITLE_COLOR));
        wattroff (curr_win->WIN, A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->WIN, y, x + i, "%c", curr_title[i]);

            if (key_color_toggle) {
                wattron (curr_win->WIN, COLOR_PAIR(WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            if (curr_title[i] == '(') {
                wattron (curr_win->WIN, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (curr_win->WIN);
    }

    curr_win = NULL;
    curr_title = NULL;

    return A_OK;
}

