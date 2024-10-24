#include <string.h>
#include <stdlib.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/display_lines.h"
#include "../src/utilities.h"
#include "../src/update_window_data/_update_window_data.h"

#define ASM_PATH  "mock.asm_out"



int
main (void)
{
    int ch;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[Asm] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Asm]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Asm]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    state->plugins[Asm]->has_window = true;

    window_t *win = state->plugins[Asm]->win;
    win->buff_data->changed = true;
    win->buff_data->buff = create_buff_from_file (ASM_PATH);
    state->debugger->curr_Asm_line = 1;

    win->cols = 50;
    win->rows = 30;
    win->y = 8;
    win->x = 8;

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

    // display data
    display_lines (ROW_DATA, Asm, state);
    while ((ch = getch()) != 'q') {
        display_lines (ch, Asm, state);
    }

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin ();

    free (win->buff_data->buff);

    return 0;
}
