
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

static int       key_to_index (int);
static plugin_t *get_plugin (int, plugin_t*);
static void      toggle_select_window (window_t*, char*);
static void     *pulse_thread (void*) ;
static void      read_debugger_output_to_file (debug_state_t *dstate);

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
int run_plugin (int      input_key,
                state_t *state)
{
    int        plugin_index,
               func_index,
               result;
    layout_t  *layout = NULL;
    plugin_t  *plugin = NULL;
    pthread_t  thread;

    layout       = state->curr_layout;
    plugin_index = key_to_index (input_key);
    func_index   = key_function_index [plugin_index];
    plugin       = get_plugin (input_key, state->plugins);

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
        tdata.win = state->curr_layout->header;
        result = pthread_create (&thread, NULL, pulse_thread, (void*)&tdata);
        if (result != 0) {
            pfeme ("Thread create error");
        }
    }

    // run plugin
    if (plugin_func_arr [func_index] (state->debug_state) == -1) {
        pfeme ("Unable to run function index %d with key \"%c\"\n", 
                func_index, input_key);
    }

    // write debugger output to file
    read_debugger_output_to_file (state->debug_state);

    // TODO: parse output

    // TODO: update windows

    return 0;
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
   Pulse thread
*/
static void      pulse_header_title_string (WINDOW *win, char *title);
    //
static void *pulse_thread (void *args) 
{
    tdata_t *data =  (tdata_t*) args;
    pulse_header_title_string (data->win, data->title);
    pthread_exit (NULL);
    return NULL;
}



static void find_window_string (WINDOW *window, char *string, int *y, int *x);
    //
    //
/*
    Switch plugin header string's colors in Ncurses WINDOW element to indicate usage
    ---------
    - Used by pulse_thread()
*/
static void pulse_header_title_string (WINDOW *win,
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
    Toggle select window
    -------------------
    win   = window_t struct pointer
    title = window title string
*/
static void toggle_select_window (window_t *win,
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
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates if found
    - Otherwise set both to -1
    - Used by pulse_header_title_string(), toggle_select_window()
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
        *y = -1;
        *x = -1;
    }

}



/*
    Reader debugger output
*/
static void read_debugger_output_to_file (debug_state_t *dstate)
{
    char    debug_out_buffer [256];
    size_t  bytes_read;
    FILE   *out_file_ptr;

    // open output file
    out_file_ptr = fopen(dstate->out_file_path, "w");

    // read debugger output to file
    while (1) 
    {
        bytes_read = read (dstate->output_pipe, debug_out_buffer, sizeof (debug_out_buffer) - 1);
        debug_out_buffer [bytes_read] = '\0';
        fprintf (out_file_ptr, "%s", debug_out_buffer);

        // break on string indicating output finished  (e.g. "(gdb)"
        if (strstr (debug_out_buffer, dstate->out_done_str) != NULL) {
            break;
        }
    }

    // close file
    fclose(out_file_ptr);
}
