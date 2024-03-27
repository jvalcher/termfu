
/*
    Run plugin
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "run_plugin.h"
#include "data.h"
#include "render_layout.h"
#include "utilities.h"

#include "plugins/_plugins.h"
#include "plugins/termide.h"
#include "plugins/gdb.h"



plugin_t    *get_plugin (int key, layout_t *layout);
static void  pulse_header_string_on  (char*, WINDOW*, int*, int*);
static void  pulse_header_string_off (char*, WINDOW*, int*, int*, double);
static bool  check_is_window (int, plugin_t*);



/*
    Run plugin function
    --------
    - Match key input (main.c) to plugin code (render_layout.c) in current layout
      set in CONFIG_FILE (parse_config.c, data.h)
    - Run plugin code's corresponding function (plugins/...) via its pointer (above)
    - Pulse corresponding header title string's colors
        - Minimum time of MIN_PULSE_LEN seconds
        - Otherwise color switches back after plugin function returns
*/
int run_plugin (int            input_key,
                layout_t      *layout)
{
    int     result,
            plugin_index,
            func_index,
            y, x;
    char   *code;
    struct  timeval start, end;
    double  func_time;
    plugin_t* plugin;

    plugin_index = key_to_index (input_key);
    func_index   = key_function_index [plugin_index];
    code         = plugin_code [func_index];
    plugin       = get_plugin (input_key, layout);

    // switch header string color
    if (plugin->window == NULL) {
        gettimeofday (&start, NULL);
        pulse_header_string_on (plugin->title, layout->header, &y, &x);
    }

    // run plugin
    if (plugin_func_arr [func_index] (plugin) == -1) {
        pfeme ("Unable to run function index %d with key \"%c\"\n", 
                func_index, input_key);
    }

    // switch header string color back
    if (plugin->window == NULL) {
        gettimeofday (&end, NULL);
        func_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
        pulse_header_string_off (plugin->title, layout->header, &y, &x, func_time);
    }

    return 1;
}



/*
    Get plugin associated with input key
*/
plugin_t *get_plugin (int key,
                  layout_t *layout)
{
    plugin_t *curr_plugin = layout->plugins;
    do {
        if (key == curr_plugin->key) {
            return curr_plugin;
        }
        curr_plugin = curr_plugin->next;
    } while (curr_plugin != NULL);
    return NULL;
}



/*
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates if found
    - Otherwise set both to -1
*/
void find_window_string (WINDOW *window,
                         char   *string,
                         int    *y,
                         int    *x)
{
    int  i, j, 
         m, n,
         ch,
         si = 0, 
         rows, cols;
    bool found = false;

    // get number of rows, columns
    getmaxyx (window, rows, cols);

    // find string
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            ch = mvwinch (window, i, j);
            if ((char) ch == string [si]) {
                if (si == 0) {
                    m = i;
                    n = j;
                }
                si += 1;
                if (si == strlen (string)) {
                    found = true;
                    break;
                }
            } else {
                si = 0;
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
    Switch plugin header string's colors in Ncurses WINDOW element to indicate usage
*/
static void pulse_header_string_on (char   *title,
                                    WINDOW *header,
                                    int *y,
                                    int *x)
{
    int i;
    bool key_color_toggle;

    find_window_string (header, title, y, x);

    // switch colors
    if (*y != -1) {

        key_color_toggle = false;

        // reverse colors
        wattron (header, COLOR_PAIR(FOCUS_HEADER_TITLE_COLOR));
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (header, *y, *x + i, "%c", title [i]);
            if (key_color_toggle) {
                wattron (header, COLOR_PAIR(FOCUS_HEADER_TITLE_COLOR));
                key_color_toggle = false;
            }
            if (title [i] == '(') {
                wattron (header, COLOR_PAIR(FOCUS_TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh (header);

    }
}



/*
    Reverse code string colors back to normal after plugin function returns or 
    after MIN_PULSE_LEN seconds
*/
static void pulse_header_string_off (char   *title,
                                     WINDOW *header,
                                     int *y,
                                     int *x,
                                     double  func_time)
{
    int  i;
    bool key_color_toggle;

    // switch colors
    if (*y != -1) {

        key_color_toggle = false;

        // sleep if plugin function call took less than MIN_PULSE_LEN seconds
        // to make sure color pulse is visible
        if (func_time < MIN_PULSE_LEN)
            usleep (MIN_PULSE_LEN * 1000000);

        // switch colors back
        wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (header, *y, *x + i, "%c", title[i]);
            if (key_color_toggle) {
                wattron (header, COLOR_PAIR(HEADER_TITLE_COLOR));
                key_color_toggle = false;
            }
            if (title[i] == '(') {
                wattron (header, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh  (header);
    }
}



/*
    Check if key corresponds to a plugin_t struct with
    a window_t struct attached
*/
bool check_is_window (int key, plugin_t *plugins) {
    plugin_t *curr_plugin = plugins;
    do {
        if (key == curr_plugin->key) {
            return true;
        }
    } while (curr_plugin != NULL);
    return false;
}
