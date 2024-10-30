#include <string.h>
#include <stdlib.h>

#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/display_lines.h"
#include "../src/utilities.h"
#include "test_utilities.h"

#define MOCK_ASM_PATH  "/home/jvalcher/Git/termfu/tests/mock.asm_out"



int
main (void)
{

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

    //////////// create parent, data window

    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
    refresh ();

    win->cols = 64;
    win->rows = 32;
    win->y = 4;
    win->x = 8;

    create_ncurses_data_window (win);

    ////////////


    int ch;
    buff_data->buff         = create_buff_from_file (MOCK_ASM_PATH);
    plugin->has_window      = true;
    buff_data->text_wrapped = false;
    buff_data->changed      = true;
    debugger->curr_Asm_line = 26;

    display_lines (plugin->data_pos, plugin_index, state);

    while ((ch = getch()) != 'q') {

        display_lines (ch, plugin_index, state);
        
    }


    //////////// close ncurses

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin ();

    ////////////

    return 0;
}

