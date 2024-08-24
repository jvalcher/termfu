#include <string.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/display_lines.h"
#include "../src/update_window_data/_update_window_data.h"
#include "../src/utilities.h"

#define PROG_PATH  "./src/convert.c"



int
main (void)
{
    int ch;
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Src] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Src]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Src]->win->file_data = (file_data_t*) malloc (sizeof (file_data_t));
    window_t *win = state->plugins[Src]->win;

    win->file_data->path_changed = true;
    win->file_data->line = 27;
    strcpy (win->file_data->path, PROG_PATH);

    win->rows = 30;
    win->cols = 50;
    win->y = 20;
    win->x = 20;

    win->data_win_rows = win->rows - 2;
    win->data_win_cols = win->cols - 2;
    win->data_win_y = 1;
    win->data_win_x = 1;

    // start ncurses
    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
    refresh ();

    // create ncurses windows
        // parent
    win->WIN = newwin (win->rows, win->cols, win->y, win->x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);

        // data win
    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, win->data_win_y, win->data_win_x);
    wrefresh (win->DWIN);

    display_lines (FILE_TYPE, LINE_DATA, win);

    while ((ch = getch()) != 'q') {

        if (ch == 'L') {
            display_lines (FILE_TYPE, LINE_DATA, win);
        }

        display_lines (FILE_TYPE, ch, win);
        
    }

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin ();

    return 0;
}
