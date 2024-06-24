
/*
    Debugger reader process window update
*/

#include <string.h>

#include "render_window_data.h"
#include "update_window_data.h"
#include "data.h"
#include "utilities.h"

static window_t  *calculate_window_data  (plugin_t*, char*);



void update_window_data (state_t *state, 
                         char *code)
{
    window_t *win = calculate_window_data (state->plugins, code);

    render_window_data (win, state, BEGINNING, WINDOW_DATA);
}



static window_t *calculate_window_data (plugin_t *plugins, 
                                        char *code)
{
    plugin_t *plugin;
    window_t *win;
    FILE     *fp;
    int       i, ch;

    // find code's plugin
    plugin = plugins;
    do {
        if (strcmp (plugin->code, code) == 0)
            break;
        plugin = plugin->next;
    } while (plugins != NULL);
    if (plugin == NULL){
        pfeme ("Unable to find plugin for code \"%s\"", code);
    }
    win = plugin->window;

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
    win->win_rows -= 2;
    win->win_cols -= 2;

    // calculate min, max middle file lines for vertical scrolling
    win->file_min_mid = (win->win_rows / 2) + 1;
    win->file_max_mid = win->file_rows - ((win->win_rows - 1) / 2);

    // set initial window middle line
    win->win_mid_line = win->file_min_mid;

    // set first char to display for horizontal scrolling
    win->file_first_char = 0;

    fclose (fp);

    return win;
}



