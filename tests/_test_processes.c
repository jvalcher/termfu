/*

    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Src;

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



    //////////// create ncurses parent, data window

    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
    refresh ();

    win->cols = 64;
    win->rows = 32;
    win->y = 8;
    win->x = 8;

    create_ncurses_data_window (win);

    ////////////



    //////////// close ncurses

    keypad (stdscr, FALSE);
    curs_set(1);
    endwin ();

    ////////////

*/
