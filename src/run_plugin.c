
/*
    Run plugin
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>

#include "run_plugin.h"
#include "data.h"
#include "utilities.h"
#include "bind_keys_windows.h"
#include "plugins/_plugins.h"


#define PULSE_LEN  .06

static int   key_to_index (int);
plugin_t    *get_plugin (int, layout_t*);
static void  pulse_header_title_string (WINDOW*, char*);
static void  toggle_select_window (window_t*, char*);
void        *pulse_thread (void*) ;

window_t *curr_win;
char      curr_title [MAX_TITLE_LEN];
typedef struct thread_data {
    WINDOW *win;
    char    title [MAX_TITLE_LEN];
} tdata_t;
tdata_t tdata;


/*
    Run plugin function
    --------
    - Match key input to plugin code in current layout  (key_bindings.h)
    - Run plugin code's corresponding function via its pointer  (plugins/_plugins.c) 
    - Pulse corresponding header title string's colors for PULSE_LEN
*/
int run_plugin (int       input_key,
                layout_t *layout)
{
    int       plugin_index,
              func_index,
              result;
    struct    timeval start;
    plugin_t *plugin;
    pthread_t thread;
    bool      thread_created = false;

    // get plugin information
    plugin_index = key_to_index (input_key);
    func_index   = key_function_index [plugin_index];
    plugin       = get_plugin (input_key, layout);

    // unselect curr_win if set
    if (curr_win != NULL && curr_win != plugin->window) {
        toggle_select_window (curr_win, curr_title);
    }

    // toggle window color or pulse header title string color
    if (plugin->window) {
        strncpy (curr_title, plugin->title, MAX_TITLE_LEN - 1);
        toggle_select_window (plugin->window, plugin->title);
    } else {
        strncpy (tdata.title, plugin->title, MAX_TITLE_LEN - 1);
        tdata.win = layout->header;
        result = pthread_create (&thread, NULL, pulse_thread, (void*)&tdata);
        if (result != 0) {
            pfeme ("Thread create error");
        }
        thread_created = true;
    }

    // run plugin
    if (plugin_func_arr [func_index] (plugin) == -1) {
        pfeme ("Unable to run function index %d with key \"%c\"\n", 
                func_index, input_key);
    }

    // join pulse thread
    if (thread_created) {
        result = pthread_join(thread, NULL);
        if (result != 0) {
            pfeme ("Thread create error");
        }
    }

    return 0;
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
   Pulse thread
*/
void *pulse_thread (void *args) 
{
    tdata_t *data =  (tdata_t*) args;
    pulse_header_title_string (data->win, data->title);
    pthread_exit (NULL);
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
    Toggle select window
    -------------------
    win   = window_t struct pointer
    title = window title string
*/
static void 
toggle_select_window (window_t *win,
                      char     *title) 
{
    int i, x, y;
    bool key_color_toggle;
    bool selected = win->selected;

    // find window string
    find_window_string (win->win, title, &y, &x);

    // toggle window title color
    if (y != -1) {

        key_color_toggle = false;

        // unselect window
        if (selected) {

            wattron (win->win, COLOR_PAIR(WINDOW_TITLE_COLOR));
            wattroff (win->win, A_UNDERLINE);
            for (i = 0; i < strlen (title) + 1; i++) {
                mvwprintw (win->win, y, x + i, "%c", title[i]);
                if (key_color_toggle) {
                    wattron (win->win, COLOR_PAIR(WINDOW_TITLE_COLOR));
                    key_color_toggle = false;
                }
                if (title[i] == '(') {
                    wattron (win->win, COLOR_PAIR(TITLE_KEY_COLOR));
                    key_color_toggle = true;
                }
            }
            curr_win = NULL;

        // select window
        } else {

            wattron (win->win, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR) | A_UNDERLINE);
            for (i = 0; i < strlen (title) + 1; i++) {
                mvwprintw (win->win, y, x + i, "%c", title [i]);
                if (key_color_toggle) {
                    wattron (win->win, COLOR_PAIR(FOCUS_WINDOW_TITLE_COLOR));
                    key_color_toggle = false;
                }
                if (title [i] == '(') {
                    wattron (win->win, COLOR_PAIR(FOCUS_WINDOW_TITLE_KEY_COLOR));
                    key_color_toggle = true;
                }
            }
            curr_win = win;
        }
    }

    wrefresh  (win->win);

    // toggle selected value
    win->selected = win->selected ? false : true;
}



/*
    Switch plugin header string's colors in Ncurses WINDOW element to indicate usage
*/
static void 
pulse_header_title_string (WINDOW *win,
                           char   *title)
{
    int i, x, y;
    bool key_color_toggle;

    find_window_string (win, title, &y, &x);

    if (y != -1) {


        // pulse on
        key_color_toggle = false;
        wattron (win, COLOR_PAIR(FOCUS_HEADER_TITLE_COLOR));
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (win, y, x + i, "%c", title [i]);
            if (key_color_toggle) {
                wattron (win, COLOR_PAIR(FOCUS_HEADER_TITLE_COLOR));
                key_color_toggle = false;
            }
            if (title [i] == '(') {
                wattron (win, COLOR_PAIR(FOCUS_TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh (win);

        // pause before pulse off
        usleep (PULSE_LEN * 2000000);

        // pulse off
        key_color_toggle = false;
        wattron (win, COLOR_PAIR(HEADER_TITLE_COLOR));
        for (i = 0; i < strlen (title) + 1; i++) {
            mvwprintw (win, y, x + i, "%c", title[i]);
            if (key_color_toggle) {
                wattron (win, COLOR_PAIR(HEADER_TITLE_COLOR));
                key_color_toggle = false;
            }
            if (title[i] == '(') {
                wattron (win, COLOR_PAIR(TITLE_KEY_COLOR));
                key_color_toggle = true;
            }
        }
        wrefresh (win);
    }
}



/* 
    Convert key stroke character to plugin function index
*/
static int key_to_index (int key)
{
    if (key >= 'a' && key <= 'z') {
        return key - 'a' + 1;
    }
    else if (key >= 'A' && key <= 'Z') {
        return key - 'A' + 27;
    } else {
        return -1;
    }
}


