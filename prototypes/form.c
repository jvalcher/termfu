#include <ncurses.h>

#define BUFF_LEN  256



int
main (void)
{
    WINDOW *w1;
    WINDOW *w1_data;
    WINDOW *w2;
    WINDOW *w3;
    WINDOW *w4;
    WINDOW *curr_win;
    WINDOW *popup_parent;
    WINDOW *popup_data;
    
    char  input_buffer [BUFF_LEN];
    char *input_prompt = "Input: ";

    initscr();
    curs_set(0);
    noecho();

    // create windows
    w1 = newwin (10, 20, 1, 1);
    box (w1, 0, 0);
    w1_data = derwin(w1, 8, 18, 1, 1);
    w2 = newwin (10, 20, 11, 1);
    box (w2, 0, 0);
    w3 = newwin (10, 20, 1, 21);
    box (w3, 0, 0);
    w4 = newwin (10, 20, 11, 21);
    box (w4, 0, 0);
    refresh();
    wrefresh(w1);
    wrefresh(w1_data);
    wrefresh(w2);
    wrefresh(w3);
    wrefresh(w4);

    getch();

    // open popup window
    curs_set(1);
    echo();
    doupdate();
    curr_win = dupwin (curscr);

        // create popup parent window
    popup_parent = newwin(10, 20, 6, 6);
    box(popup_parent, 0, 0);
    wrefresh(popup_parent);

        // create popup input window
    popup_data = derwin(popup_parent, 8, 18, 1, 1);
    mvwprintw(popup_data, 0, 0, "%s", input_prompt);
    wrefresh(popup_data);

        // get input
    wgetnstr(popup_data, input_buffer, BUFF_LEN - 1);

        // close popup
    touchwin(curr_win);
    wnoutrefresh(curr_win);
    doupdate();
    delwin(curr_win);
    curs_set(0);
    noecho();

    // print input to screen
    mvwprintw(w1_data, 0, 0, "%s", input_buffer);
    wrefresh(w1_data);

    getch();

    endwin();
    return 0;
}
