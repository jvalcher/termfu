
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <ncurses.h>
#include "../utilities.h"
#include "../data.h"



/*
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates if found
    - Otherwise set both to -1
*/
void find_window_string (WINDOW *window,
                         char *string,
                         int *y,
                         int *x)
{
    int i, j, 
        m, n,
        ch,
        c = 0, 
        rows, cols;
    bool found = false;

    // get number of rows, columns
    getmaxyx (window, rows, cols);

    // find string
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            ch = mvwinch(window, i, j);
            if ((char) ch == string [c]) {
                if (c == 0) {
                    m = i;
                    n = j;
                }
                c += 1;
                if (c == strlen (string)) {
                    found = true;
                    break;
                }
            } else {
                c = 0;
            }
        }
        if (found) break;
    }

    // set y,x to coordinates or -1
    if (found) {
        *y = m;
        *x = n;
    } else {
        *y = -1;
        *x = -1;
    }

}



/*
    Get title string from plugin code for current layout
    ----------
    Nxt -> (n)ext
*/
char *get_code_title (char *code,
                      layout_t *layout)
{
    plugin_t *curr_plugin = layout->plugins;
    do {
        if (strcmp (code, curr_plugin->code) == 0) {
            return (char*) curr_plugin->title;
        }
        curr_plugin = curr_plugin->next;
    } while (curr_plugin != NULL);
    return NULL;
}



/*
    Switch string's colors in Ncurses WINDOW element for quarter second
    ------------
    - Used at beginning of plugin functions with title string in header 
      to indicate usage
*/
void pulse_window_string (char *code,
                          layout_t *layout)
{
    int y, x, i;
    bool key_color_toggle;
    char *title;
    WINDOW *window;

    // get header title string for current layout
    title = get_code_title (code, layout);
    if (title == NULL)  {
        pfeme ("Unable to find \"%s\" title for layout \"%s\"", code, layout->label);
    }

    // find string coordinates (y,x)
    window = layout->header;
    find_window_string (window, title, &y, &x);

    // switch colors
    if (y != -1) {

        key_color_toggle = false;

        // reverse colors
        wattron (window, COLOR_PAIR(FOCUS_HEADER_TITLE_COLOR));
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (window, y, x + i, "%c", title [i]);
            if (key_color_toggle) {
                wattron (window, COLOR_PAIR(FOCUS_HEADER_TITLE_COLOR));
                key_color_toggle = false;
            }
            if (title [i] == '(') {
                wattron (window, COLOR_PAIR(FOCUS_TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (window);

        // sleep quarter second
        usleep (100000);

        // switch colors back
        wattron (window, COLOR_PAIR(HEADER_TITLE_COLOR));
        for (i = 0; i < strlen(title) + 1; i++) {
            mvwprintw (window, y, x + i, "%c", title[i]);
            if (key_color_toggle) {
                wattron (window, COLOR_PAIR(HEADER_TITLE_COLOR));
                key_color_toggle = false;
            }
            if (title[i] == '(') {
                wattron (window, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (window);

    } else {
        pfeme  ("Unable to locate window string \"%s\"\n", title);
    }
}



