#include <stdlib.h>

#include "test_utilities.h"
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/display_lines.h"
#include "../src/utilities.h"

#define ASM_PATH  "mock.asm_out"



int
main (void)
{
    int ch;

    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Asm;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    buff_data_t *buff_data = win->buff_data;

    ////////////

    plugin->has_window      = true;
    win->buff_data->changed = true;
    win->buff_data->buff = create_buff_from_file (ASM_PATH);
    buff_data->text_wrapped = false;
    buff_data->changed      = true;
    debugger->curr_Asm_line = 26;

    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
    refresh ();

    win->cols = 50;
    win->rows = 30;
    win->y = 8;
    win->x = 8;

    create_ncurses_data_window (win);

    display_lines (ROW_DATA, Asm, state);

    while ((ch = getch()) != 'q') {
        display_lines (ch, Asm, state);
    }

    keypad (stdscr, FALSE);
    curs_set (1);
    endwin ();

    return 0;
}
