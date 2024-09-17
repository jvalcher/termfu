
#include "select_window.h"
#include "data.h"
#include "display_lines.h"
#include "get_popup_window_input/popup_watchpoints.h"
#include "update_window_data/_update_window_data.h"
#include "utilities.h"
#include "plugins.h"
#include "get_popup_window_input/popup_breakpoints.h"

#define ESC        27
#define BUFF_LEN  256

static void  select_window_color    (int plugin_index, state_t *state);
static void  deselect_window_color  (void);

char     *curr_title = NULL;
window_t *curr_win   = NULL;
WINDOW   *curr_WIN   = NULL;



void
select_window (int      plugin_index,
               state_t *state)
{
    int           key,
                  type;
    bool          in_loop         = true,
                  key_not_pressed = true;

    type = state->plugins[plugin_index]->win_type;

    select_window_color (plugin_index, state);

    keypad (stdscr, TRUE);

    while (in_loop) {

        key = getch ();

        // TODO: pg up/down, home, end
        switch (key) {
            case 'k':
            case KEY_UP:
                display_lines (type, KEY_UP, plugin_index, state);
                key_not_pressed = false;
                break;
            case 'j':
            case KEY_DOWN:
                display_lines (type, KEY_DOWN, plugin_index, state);
                key_not_pressed = false;
                break;
            case 'l':
            case KEY_RIGHT:
                display_lines (type, KEY_RIGHT, plugin_index, state);
                key_not_pressed = false;
                break;
            case 'h':
            case KEY_LEFT:
                display_lines (type, KEY_LEFT, plugin_index, state);
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

            // window commands
            switch (plugin_index) {

                case Brk:
                    switch (key) {
                        case 'd': 
                            delete_breakpoint (state);
                            in_loop = false;
                            break;
                        case 'c':
                            insert_breakpoint (state);
                            in_loop = false;
                            break;
                        case 'a':
                            clear_all_breakpoints (state);
                            in_loop = false;
                            break;
                    }
                    break;

                case Wat:
                    switch (key) {
                        case 'd':
                            delete_watchpoint (state);
                            in_loop = false;
                            break;
                        case 'c':
                            insert_watchpoint (state);
                            in_loop = false;
                            break;
                        case 'a':
                            clear_all_watchpoints (state);
                            in_loop = false;
                            break;
                        }
                    }
                    break;
        }
        key_not_pressed = true;
    }

    deselect_window_color ();

    keypad (stdscr, FALSE);
}



// TODO: change top, right, left borders color
// - corrected corner characters stored in state->plugins[x]->win->border

/*
    Change Ncurses window title color to indicate focus
*/
static void
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

    string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x);

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
}



static void
deselect_window_color (void)
{

    size_t i;
    int x, y;
    bool key_color_toggle,
         string_exists;

    string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x);

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
}

