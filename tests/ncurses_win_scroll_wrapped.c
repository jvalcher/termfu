#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <stdlib.h>

#include "test_utilities.h"
#include "../src/data.h"
#include "../src/utilities.h"
#include "../src/display_lines.h"

#define BUF_LEN  8192
#define ESC      27



int
main (void)
{
    int     ch, i,
            y, x,
            scr_rows, scr_cols;
    bool    running;
    FILE   *fp;

    window_t *win = (window_t*) malloc (sizeof (window_t));
    win->data_win_cols = 48;
    win->data_win_rows = 28;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    buff_data_t *buff_data = win->buff_data;
    buff_data->buff = malloc (BUF_LEN);
    buff_data->scroll_row = 1;      // beginning data

    //process_pause();

    // create buffer
    fp = fopen ("../src/main.c", "r");
    for (i = 0; i < BUF_LEN; i++) {
        ch = fgetc (fp);
        if (ch == EOF) {
            buff_data->buff [i] = '\0';
            break;
        }
        buff_data->buff [i] = (char) ch;
    }
    fclose (fp);

    // allocate llist
    create_scroll_buffer_llist (win);

    /*
    buff_line = buff_data->head;
    do {
        printf ("(%d) %.*s\n", buff_line->line, buff_line->len, buff_line->ptr);
        buff_line = buff_line->next;
    } while (buff_line != NULL);
    */

    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair (1, COLOR_WHITE, COLOR_BLUE);
    init_pair (2, COLOR_BLACK, COLOR_GREEN);
    keypad (stdscr, TRUE);
    curs_set (0);
    refresh();

    getmaxyx (stdscr, scr_rows, scr_cols);
    y = (scr_rows - (win->data_win_rows+2)) / 2;
    x = (scr_cols - (win->data_win_cols+2)) / 2;

    win->WIN = newwin (win->data_win_rows+2, win->data_win_cols+2, y, x);
    box (win->WIN, 0, 0);
    wrefresh (win->WIN);

    win->DWIN = derwin (win->WIN, win->data_win_rows, win->data_win_cols, 1, 1);
    wrefresh (win->DWIN);

    display_scroll_buff_lines (BEG_DATA, win);

    running = true;
    while (running) {

        ch = getch ();

        if (ch == 'q' || ch == ESC) {
            running = false;
            break;
        }

        display_scroll_buff_lines (ch, win);
    }

    curs_set (1);
    endwin();

    return 0;
}
