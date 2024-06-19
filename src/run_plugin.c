
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
#include "plugins/_interface.h"
#include "utilities.h"
#include "bind_keys_windows.h"
#include "plugins/_plugins.h"

static int       key_to_index               (int);
static plugin_t *get_plugin                 (int, plugin_t*);
static void      pulse_header_title         (state_t*);
static void     *start_pulse_header_thread  (void *args);
       void      select_window              (char*, window_t*);
       void      deselect_window            (char*, window_t*);
static void      find_window_string         (WINDOW*, char*, int*, int*);

char      curr_title [MAX_TITLE_LEN];
window_t *curr_win = NULL;

typedef struct thread_data {
    WINDOW *win;
    char    title [MAX_TITLE_LEN];
} tdata_t;



/*
    Run plugin function
    --------
    - Match key input to plugin code in current layout
    - Indicate usage via the plugin's title string in the header or its window
    - Run plugin code's function via its pointer
*/
void run_plugin (int      key,
                 state_t *state)
{
    int plugin_index   = key_to_index (key);
    if (plugin_index != -1) {
        
        int func_index     = key_function_index [plugin_index];
        state->curr_plugin = get_plugin (key, state->plugins);
        state->curr_window = state->curr_plugin->window;

        /*
        if (key < 'A' || (key > 'Z' && key < 'a') || key > 'z')
            run_non_plugin_key (key, state);
        */

        if (state->curr_plugin->window)
            select_window (state->curr_plugin->title, state->curr_window);

        else
            pulse_header_title (state);

        insert_output_start_marker (state);
        plugin_func [func_index]   (state);
        insert_output_end_marker   (state);
    }
}



/* 
    Convert key letter to plugin function index
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



/*
    Get plugin associated with input key
*/
static plugin_t *get_plugin (int key,
                             plugin_t *plugins)
{
    plugin_t *curr_plugin = plugins;
    do {
        if (key == curr_plugin->key) {
            return curr_plugin;
        }
        curr_plugin = curr_plugin->next;
    } while (curr_plugin != NULL);
    return NULL;
}



/*
    Pulse header title string color
    --------
    - Indicator for plugin function being called
*/        
void pulse_header_title (state_t *state)
{
    tdata_t    tdata;
    strncpy (tdata.title, plugin->title, MAX_TITLE_LEN - 1);
    tdata.win = state->curr_layout->header;
    result = pthread_create (&thread, NULL, start_pulse_header_thread, (void*)&tdata);
    if (result != 0) {
        pfeme ("Thread create error");
    }
}



static void *start_pulse_header_thread (void *args) 
{
    float pulse_len;
    tdata_t *data =  (tdata_t*) args;
    int i, x, y;
    bool key_color_toggle;
    char *title = data->title;
    WINDOW *win = data->win;
    
    pulse_len = .06;

    find_window_string (win, title, &y, &x);

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
    usleep (pulse_len * 2000000);

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

    pthread_exit (NULL);
}



void select_window (char *title, window_t *win)
{
    int i, x, y;
    bool key_color_toggle;

    if (curr_win)
        deselect_window (title, curr_win);
    curr_win = win;

    find_window_string (win->win, title, &y, &x);

    win->selected = true;

    key_color_toggle = false;
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
    wrefresh  (win->win);
}



void deselect_window (char *title, window_t *win)
{
    int i, x, y;
    bool key_color_toggle;
    window_t *win = plugin->window;

    find_window_string (win->win, plugin->title, &y, &x);

    win->selected = false;

    key_color_toggle = false;
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
    wrefresh  (win->win);
}



/*
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates
*/
static void find_window_string (WINDOW *window,
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
        pfeme ("Unable to find window string \"%s\"", string);
    }

}

