
#include "test_utilities.h"
#include "../src/data.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"
#include "../src/utilities.h"
#include "../src/get_form_input/watchpoints.h"



int
main (void)
{
    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Wat;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    //debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    //buff_data_t *buff_data = win->buff_data;

    ////////////

    plugin->has_window = true;

    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    refresh ();

    win->rows = 30;
    win->cols = 50;
    win->y = 8;
    win->x = 20;

    create_ncurses_data_window (win);

    char *cmd[] = {"gdb", "--quiet", "--interpreter=mi", "../misc/hello"};
    state->command = cmd;

    start_debugger (state);

    send_command_mp (state, "-break-insert 16\n");
    send_command_mp (state, "-exec-run\n");

    // variables: n1,  n_arr,  n_len,  str1,  str2,  str3
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
