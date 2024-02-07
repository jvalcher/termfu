
/*
    Run plugin
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "run_plugin.h"
#include "data.h"
#include "render_layout.h"
#include "utilities.h"

#include "plugins/termide.h"
#include "plugins/gdb.h"


/*
    Plugin codes
    -----------
    - Indexes match corresponding function's index in plugin_function[] array  (run_plugin.c)
    - Ordered alphabetically for binary search in bind_keys_to_plugin()
    - Used to bind function index to shortcut key in key_function_index[]  (data.h)
*/
char *plugin_code [] = {
    
    "EMP",
    "Asm",
    "Bak",
    "Bld",
    "Brk",
    "Con",
    "Fin",
    "Kil",
    "Lay",
    "LcV",
    "Nxt",
    "Out",
    "Prm",
    "Prn",
    "Reg",
    "Run",
    "Src",
    "Stp",
    "Wat"
};

/*
    Function pointer array
    ----------
    - Indexes match corresponding plugin code string in plugin_code[]
        - Ordered alphabetically by code
    - Indexes set in key_function_index[]  (data.h)  by bind_keys_to_plugins()  (render_layout.c)
    - Functions called in main() loop  (main.c)
*/
typedef int (*plugin_func_t) (layout_t *layout);
    //
plugin_func_t plugin[] = {

    (plugin_func_t) empty_func,         // "EMP"
    (plugin_func_t) gdb_assembly,       // "Asm"
    (plugin_func_t) termide_back,       // "Bak"
    (plugin_func_t) termide_builds,     // "Bld"
    (plugin_func_t) gdb_breakpoints,    // "Brk"
    (plugin_func_t) gdb_continue,       // "Con"
    (plugin_func_t) gdb_finish,         // "Fin"
    (plugin_func_t) gdb_kill,           // "Kil"
    (plugin_func_t) termide_layouts,    // "Lay"
    (plugin_func_t) gdb_local_vars,     // "LcV"
    (plugin_func_t) gdb_next,           // "Nxt"
    (plugin_func_t) gdb_output,         // "Out"
    (plugin_func_t) gdb_prompt,         // "Prm"
    (plugin_func_t) gdb_print,          // "Prn"
    (plugin_func_t) gdb_registers,      // "Reg"
    (plugin_func_t) gdb_run,            // "Run"
    (plugin_func_t) gdb_src_file,       // "Src"
    (plugin_func_t) gdb_step,           // "Stp"
    (plugin_func_t) gdb_watches         // "Wat"
};

#define MIN_PULSE_LEN  .06

static void pulse_window_string_on  (char*, layout_t*);
static void pulse_window_string_off (char*, layout_t*, double);



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
int run_plugin (int       input_key,
                layout_t *layout)
{
    int     result;
    int     plugin_index;
    int     function_index;
    char   *code;
    struct  timeval start, end;
    double  func_time;

    // get plugin function index and code
    plugin_index   = key_to_index (input_key);
    function_index = key_function_index [plugin_index];
    code           = plugin_code [function_index];

    // switch header string color
    gettimeofday (&start, NULL);
    pulse_window_string_on (code, layout);

    // run plugin
    result = plugin [function_index] (layout);
    if (result == -1) {
        pfeme ("Unable to run function index %d with key \"%c\"\n", 
                function_index, input_key);
    }

    // switch color back
    gettimeofday (&end, NULL);
    func_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    pulse_window_string_off (code, layout, func_time);

    return 1;
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
    Get title string from plugin code for current layout
    ----------
    Nxt -> (n)ext
*/
char *get_code_title (char     *code,
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
    Switch plugin header string's colors in Ncurses WINDOW element to indicate usage
*/
static void pulse_window_string_on (char     *code,
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
        wrefresh (window);

    }
}



/*
    Reverse code string colors back to normal after plugin function returns or 
    after MIN_PULSE_LEN seconds
*/
static void pulse_window_string_off (char     *code,
                                     layout_t *layout,
                                     double    func_time)
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

        // sleep if plugin function call took less than MIN_PULSE_LEN seconds
        // to make sure color pulse is visible
        if (func_time < MIN_PULSE_LEN)
            usleep (MIN_PULSE_LEN * 1000000);

        // switch colors back
        wattron (window, COLOR_PAIR(HEADER_TITLE_COLOR));
        for (i = 0; i < strlen (title) + 1; i++) {
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
    }
}
