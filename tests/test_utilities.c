#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ncurses.h>

#include "test_utilities.h"
#include "../src/data.h"
#include "../src/error.h"
#include "../src/utilities.h"
#include "../src/plugins.h"
#include "../src/start_debugger.h"

// Default parent window size
int test_cols = 60;
int test_rows = 20;
int test_y = 10;
int test_x = 10;   



void
process_pause (void)
{
    printf (
        "\n"
        "Process ID:    \033[0;36m%ld\033[0m \n"
        "\n"
        "Connect to this process with debugger\n"
        "\n"
        "    $ make conn_proc_<debugger>\n"
        "    - Set breakpoint\n"
        "    - Continue\n"
        "\n"
        "Press any key to continue...\n"
        "\n",
        (long) getpid ());

    // wait...
    getchar ();
}



state_t *start_termfu (int argc, char *argv[])
{
    state_t *state = (state_t*) malloc (sizeof(state_t));
    state->debugger = (debugger_t*) malloc (sizeof(debugger_t));

    if (initial_configure (argc, argv, state) == FAIL)
        pfeme ("Initial configuration failed");

    if (parse_config_file (state) == FAIL)
        pfeme ("Failed to parse configuration file");

    if (start_debugger (state) == FAIL)
        pfeme ("Failed to start debugger process");

    return state;
}



state_t* allocate_test_plugin(int index)
{
    int plugin_index = index;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);

    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));

    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    state->plugins[plugin_index]->has_window = true;
    state->plugins[plugin_index]->win->buff_data->changed = true;
    state->plugins[plugin_index]->win->buff_data->text_wrapped = false;
    state->plugins[plugin_index]->win->buff_data->changed = true;
    state->debugger->curr_Asm_line = 1;
    state->debugger->curr_Src_line = 1;

    return state;
}


void init_nc (void)
{
    initscr ();
    cbreak ();
    noecho ();
    curs_set (0);
    keypad (stdscr, TRUE);
    refresh ();
}


void deinit_nc (void)
{
    keypad (stdscr, FALSE);
    curs_set (1);
    endwin ();
}


void
create_nc_window (window_t *win)
{
    // Set window sizes
        // parent
    win->rows = test_rows;
    win->cols = test_cols;
    win->y = test_y;
    win->x = test_x;
        // data
    win->data_win_rows = test_rows - 2;
    win->data_win_cols = test_cols - 2;
    win->data_win_y = 1;
    win->data_win_x = 1;

    // Create windows
        // parent
    win->WIN = newwin (win->rows, win->cols, win->y, win->x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);
        // data win
    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, win->data_win_y, win->data_win_x);
    wrefresh (win->DWIN);
}


void
logt (const char *formatted_string, ...)
{
    FILE *debug_out_test_ptr = NULL;

    if ((debug_out_test_ptr = fopen (DEBUG_OUT_TEST_PATH, "w")) == NULL)
        pfeme_errno ("Failed to open debug out file \"%s\"", DEBUG_OUT_TEST_PATH);

    va_list args;
    va_start (args, formatted_string);
    vfprintf (debug_out_test_ptr, formatted_string, args);
    va_end (args);
}


void print_debugger_state_test (state_t *state)
{
    debugger_t *dbg = state->debugger;

    logt ("title: \"%s\"\n", dbg->title);
    logt ("running: %d\n", dbg->running);
    logt ("curr_plugin_index: %d\n", dbg->curr_plugin_index);
    logt ("prog_path: \"%s\"\n", dbg->prog_path);
    logt ("main_src_path_buffer: \"%s\"\n", dbg->main_src_path_buffer);
}
