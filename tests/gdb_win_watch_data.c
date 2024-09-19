
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/get_popup_window_input/watchpoints.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/parse_debugger_output.h"
#include "../src/utilities.h"
#include "../src/update_window_data/_update_window_data.h"
#include "../src/display_lines.h"



int
main (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_num_plugins (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*) * state->num_plugins);
    state->plugins[Wat] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Wat]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Wat]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Wat_BUF_LEN);
    win->buff_data->buff_pos = 0;
    win->buff_data->buff_len = Wat_BUF_LEN;
    state->debugger->index = DEBUGGER_GDB;
    win->buff_data->new_data = true;



    state->plugins[Wat]->has_window = true;
    state->plugins[Wat]->win_type = BUFF_TYPE;
    state->plugins[Wat]->data_pos = BEG_DATA;

    win->rows = 30;
    win->cols = 50;
    win->y = 20;
    win->x = 20;

    win->data_win_rows = win->rows - 2;
    win->data_win_cols = win->cols - 2;
    win->data_win_y = 1;
    win->data_win_x = 1;

    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    refresh ();

    win->WIN = newwin (win->rows, win->cols, win->y, win->x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);

    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, win->data_win_y, win->data_win_x);
    wrefresh (win->DWIN);



    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    insert_output_start_marker (state);
    send_command (state, "-break-insert 16\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "-exec-run\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);



    // variables:  n1,  n_arr,  n_len,  str1,  str2,  str3
    insert_watchpoint (state);
    getch();
    insert_watchpoint (state);
    getch();
    insert_watchpoint (state);
    getch();

    delete_watchpoint (state);
    getch();
    delete_watchpoint (state);
    getch();

    curs_set(1);
    endwin ();

    return 0;
}
