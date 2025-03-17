
#include "select_window.h"
#include "data.h"
#include "display_lines.h"
#include "get_form_input/watchpoints.h"
#include "get_form_input/breakpoints.h"
#include "utilities.h"
#include "error.h"
#include "plugins.h"

#define BUFF_LEN  256
#define SEL_WIN_STATE  " (index: %d, code: \"%s\", key: \"%c\" (%d))", \
                        plugin_index, get_plugin_code (plugin_index), key, key

static int  select_window_color    (int plugin_index, state_t *state);
static int  deselect_window_color  (void);

char     *curr_title = NULL;
window_t *curr_win   = NULL;
WINDOW   *curr_WIN   = NULL;



int
select_window (int      plugin_index,
               state_t *state)
{
    int           key = -1;
    bool          in_loop     = true,
                  key_pressed = false;

    if (select_window_color (plugin_index, state) == FAIL)
        pfemr ("Failed to select window color" SEL_WIN_STATE);

    while (in_loop) {

        key = getch();

        switch (key) {

        // Built-in
            // Navigation
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
            if (display_lines (key, plugin_index, state) == FAIL)
                pfemr ("Failed to display lines" SEL_WIN_STATE);
            key_pressed = true;
            break;

            // Deselect window
        case ESC:
            in_loop = false;
            key_pressed = true;
            break;
        }

        // Custom
        if (in_loop && !key_pressed) {

            // Deselect window
                // key
            if (key == state->plugins[plugin_index]->key) {
                in_loop = false;
                continue;
            }
                // Qut
            else if (key == state->plugins[Qut]->key) {
                in_loop = false;
                state->debugger->running = false;
                continue;
            }

            // Window-specific
            switch (plugin_index) {

            case Brk:
                switch (key) {
                case 'd': 
                    if (delete_breakpoint (state) == FAIL)
                        pfemr ("Failed to delete breakpoint" SEL_WIN_STATE);
                    in_loop = false;
                    break;
                case 'c':
                    if (insert_breakpoint (state) == FAIL)
                        pfemr ("Failed to insert breakpoint" SEL_WIN_STATE);
                    in_loop = false;
                    break;
                case 'a':
                    if (clear_all_breakpoints (state) == FAIL)
                        pfemr ("Failed to clear all breakpoints" SEL_WIN_STATE);
                    in_loop = false;
                    break;
                }
                break;

            case Wat:
                switch (key) {
                case 'd':
                    if (delete_watchpoint (state) == FAIL)
                        pfemr ("Failed to delete watchpoint" SEL_WIN_STATE);
                    in_loop = false;
                    break;
                case 'c':
                    if (insert_watchpoint (state) == FAIL)
                        pfemr ("Failed to insert watchpoint" SEL_WIN_STATE);
                    in_loop = false;
                    break;
                case 'a':
                    if (clear_all_watchpoints (state) == FAIL)
                        pfemr ("Failed to clear all watchpoints" SEL_WIN_STATE);
                    in_loop = false;
                    break;
                }
            }
            break;
        }

        key_pressed = false;
    }

    if (deselect_window_color() == FAIL)
        pfemr ("Failed to deselect window color" SEL_WIN_STATE);

    return A_OK;
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

    if ((string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x)) == false)
        pfemr ("Failed to find window title \"%s\"", curr_title);

    if (string_exists) {

        key_color_toggle = false;
        wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR) | A_UNDERLINE);

        for (i = 0; i < strlen (curr_title) + 1; i++) {

            mvwprintw (curr_win->WIN, y, x + i, "%c", curr_title [i]);

            if (key_color_toggle) {
                wattron (curr_win->WIN, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR));
                key_color_toggle = false;
            }

            // window (t)itle character
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

    if ((string_exists = find_window_string (curr_win->WIN, curr_title, &y, &x)) == false)
        pfemr ("Failed to find window title \"%s\"", curr_title);

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

            // window (t)itle character
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

