#include <ncurses.h>
#include <form.h>

#include "test_utilities.h"

#define BUF_LEN  8192
#define ESC      27



int
main (void)
{
    int     ch, i, c,
            rows, cols, extra_rows, y, x,
            scr_rows, scr_cols,
            offset;
    bool    running;
    char    buffer [BUF_LEN] = {'\0'},
           *buff_ptr;
    FILE   *fp;
    WINDOW *win_body,
           *win_data;
    bool    last_line_reached;

    process_pause();

    // get file
    fp = fopen ("../src/main.c", "r");
    for (i = 0; i < BUF_LEN; i++) {
        ch = fgetc (fp);
        if (ch == EOF) {
            buffer [i] = '\0';
            break;
        }
        buffer [i] = (char) ch;
    }
    fclose (fp);

    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair (1, COLOR_WHITE, COLOR_BLUE);
    init_pair (2, COLOR_BLACK, COLOR_GREEN);
    keypad (stdscr, TRUE);
    refresh();

    // field dimensions
    cols = 48;
    rows = 24;
    getmaxyx (stdscr, scr_rows, scr_cols);
    y = (scr_rows - (rows+2)) / 2;
    x = (scr_cols - (cols+2)) / 2;

    win_body = newwin (rows+2, cols+2, y, x);
    box (win_body, 0, 0);
    wrefresh (win_body);

    win_data = derwin (win_body, rows, cols, 1, 1);
    wrefresh (win_data);

    buff_ptr = buffer;
    waddstr(win_data, buff_ptr);
    getyx (win_data, y, x);
    wrefresh (win_data);

    running = true;
    last_line_reached = false;

    while (running) {

        ch = getch ();

        wclear (win_data);

        switch (ch) {

            case 'q':
            case ESC:
                running = false;
                break;

            case KEY_HOME:
                buff_ptr = buffer;
                break;

            case 'j':
            case KEY_DOWN:

                if (y >= (rows - 1)) {

                    c = 0;
                    while (*(buff_ptr + c) != '\n') {
                        ++c;
                    }

                    if (c <= cols) {
                        buff_ptr += (c + 1);
                    } else {
                        buff_ptr += cols;
                    }
                }
                break;

            case 'k':
            case KEY_UP:

                if (buff_ptr != buffer) {

                    c = 0;

                    if (*(buff_ptr - 1) == '\n') {
                        if ((buff_ptr - 1) != buffer) {
                            ++c;
                            --buff_ptr;
                            offset = 2;
                        }
                    }
                            
                    else {
                        offset = 1;
                    }

                    while (*(buff_ptr - c) != '\n' &&
                            (buff_ptr - c) != buffer) {
                        ++c;
                    }
                            
                    if ((buff_ptr - 1) == buffer) {
                        buff_ptr = buffer;
                    }

                    else {
                        if ((c - offset) <= cols) {
                            buff_ptr -= (c - 1);
                        } else {
                            if (((c - offset) % cols) != 0) {
                                buff_ptr -= ((c - offset) % cols) + 1;
                            } else {
                                buff_ptr -= (cols + 1);
                            }
                        }
                    }
                }

                break;
        }
        
        wmove (win_data, 0, 0); 
        waddstr(win_data, buff_ptr);
        getyx (win_data, y, x);
        wclrtoeol (win_data);
        wrefresh (win_data);
    }

    endwin();

    return 0;
}
