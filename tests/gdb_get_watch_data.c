
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/get_popup_window_input/popup_watchpoints.h"
#include "../src/update_window_data/get_watchpoint_data.h"
#include "../src/start_debugger.h"
#include "../src/parse_cli_arguments.h"
#include "../src/insert_output_marker.h"
#include "../src/parse_debugger_output.h"
#include "../src/utilities.h"



int
main (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Wat] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Wat]->win = (window_t*) malloc (sizeof (window_t));
    state->plugins[Wat]->win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    window_t *win = state->plugins[Wat]->win;
    win->has_data_buff = true;
    
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
    refresh ();

    // create ncurses windows
        // parent
    win->WIN = newwin (win->rows, win->cols, win->y, win->x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);
        // data win
    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, win->data_win_y, win->data_win_x);
    wrefresh (win->DWIN);

    // start debugger
    parse_cli_arguments (argc, argv, state->debugger);
    start_debugger (state);
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // insert breakpoint
    insert_output_start_marker (state);
    send_command (state, "-break-insert 16\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    // run
    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_watchpoint (state);
    insert_watchpoint (state);
    insert_watchpoint (state);

    delete_watchpoint (state);
    delete_watchpoint (state);

    getch();

    curs_set(1);
    endwin ();

    return 0;
}
