
/*
    Debugger reader process window update
*/

#include <string.h>

#include "render_window_data.h"
#include "update_window_data.h"
#include "data.h"
#include "utilities.h"

#define START  0
#define END    1

static void  get_code_plugin        (state_t*, char*);
static void  handle_title_color     (state_t*, char*, int);
static void  calculate_window_data  (state_t*);



void 
update_window_data (state_t *state, 
                    char *code)
{
    plugin_t *plugin;
    window_t *window;

    get_code_plugin (state, code);

    handle_title_color (state, code, START);

    calculate_window_data (state);

    render_window_data (state->curr_plugin->window, state, BEGINNING, WINDOW_DATA);

    handle_title_color (state, code, END);
}



static void
get_code_plugin (state_t *state,
                 char    *code)
{
    plugin_t *plugin = state->plugins;
    do {
        if (strcmp (plugin->code, code) == 0) {
            state->curr_plugin = plugin;
            break;
        }
        plugin = plugin->next;
    } while (plugin != NULL);
    if (plugin == NULL) {
        pfeme ("Unable to find plugin for code \"%s\"", code);
    }
}



static void 
handle_title_color (state_t *state,
                    char    *code,
                    int      color_mode) 
{
    switch (color_mode) {
    case START:
        if (!state->curr_plugin->window) {
            render_window_data (NULL, state, NULL, HEADER_TITLE_COLOR_ON);
        }
    case END:
        if (!state->curr_plugin->window) {
            render_window_data (NULL, state, NULL, HEADER_TITLE_COLOR_OFF);
        } else {
        }
    }
    
}



static void
calculate_window_data (state_t *state)
{
    plugin_t *plugin;
    window_t *win;
    FILE     *fp;
    int       i, ch;

    win = state->curr_plugin->window;

    // open debugger output file
    fp  = fopen (win->out_file_path, "r");
    if (fp == NULL) {
        pfeme ("Unable to open output file at \"%s\"", win->out_file_path);
    }

    // get number of rows and max line length of file
    win->file_rows = 0;
    win->file_max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_len = strlen(line);
        if (win->file_max_cols < line_len) {
            win->file_max_cols = line_len + 1;
        }
        win->file_rows += 1;
    }
    rewind (fp);

    // get file line byte offsets
    if (win->file_offsets)
        free (win->file_offsets);
    win->file_offsets = malloc ((size_t) win->file_rows * sizeof(long int));
    if (win->file_offsets == NULL) {
        pfeme ("Failed to allocate offsets array");
    }
    win->file_offsets [0] = 0;
    for (i = 1; i < win->file_rows; i++) {
        while ((ch = fgetc (fp)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            win->file_offsets [i] = ftell (fp);
    }

    // subtract borders
    win->dwin_rows = win->win_rows - 2;
    win->dwin_cols = win->win_cols - 2;

    // calculate min, max middle file lines for vertical scrolling
    win->file_min_mid = (win->dwin_rows / 2) + 1;
    win->file_max_mid = win->file_rows - ((win->dwin_rows - 1) / 2);

    // set initial window middle line
    win->win_mid_line = win->file_min_mid;

    // set first char to display for horizontal scrolling
    win->file_first_char = 0;

    fclose (fp);

    return win;
}



