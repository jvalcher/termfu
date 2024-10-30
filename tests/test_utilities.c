#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ncurses.h>

#include "test_utilities.h"
#include "../src/data.h"

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


void
create_ncurses_data_window (window_t *win)
{
    win->data_win_rows = win->rows - 2;
    win->data_win_cols = win->cols - 2;
    win->data_win_y = 1;
    win->data_win_x = 1;

    // create ncurses windows
        // parent
    win->WIN = newwin (win->rows, win->cols, win->y, win->x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);

        // data win
    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, win->data_win_y, win->data_win_x);
    wrefresh (win->DWIN);
}
