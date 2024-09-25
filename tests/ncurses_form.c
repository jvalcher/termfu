/*
    Expanded version of:
    - https://gist.github.com/alan-mushi/c8a6f34d1df18574f643
*/

#include <ncurses.h>
#include <form.h>
#include <string.h>
#include <ctype.h>

#define BUF_LEN   128

static char*
trim_whitespaces (char *str)
{
    char *end;

    // trim leading space
    while(isspace(*str)) {
        str++;
    }

    // if all spaces, return
    if(*str == '\0') {
        return str;
    }

    // trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) {
        --end;
    }
    *(end+1) = '\0';

    return str;
}

int
main (void)
{
    int     ch,
            rows, cols, y, x;
    bool    running;
    char    buffer [BUF_LEN] = {'\0'};
    FORM   *form;
    FIELD  *field[7];
    WINDOW *win_body;
    char   *prompt1 = "1 >> ",
           *prompt2 = "2 >> ",
           *prompt3 = "3 >> ",
           *enter   = "(ENTER)";

    initscr();
    cbreak();
    noecho();
    start_color();
    init_pair (1, COLOR_WHITE, COLOR_BLUE);
    init_pair (2, COLOR_BLACK, COLOR_GREEN);
    refresh();

    // field dimensions
    cols = 48;
    field[0] = new_field (1, strlen(prompt1), 0, 0, 0, 0);
    field[1] = new_field (1, cols - strlen(prompt1), 0, strlen(prompt1), 1, 0);  // one offscreen row
    field[2] = new_field (2, strlen(prompt2), 2, 0, 0, 0);
    field[3] = new_field (2, cols - strlen(prompt2), 2, strlen(prompt2), 0, 0);
    field[4] = new_field (2, strlen(prompt3), 5, 0, 0, 0);
    field[5] = new_field (2, cols - strlen(prompt3), 5, strlen(prompt3), 0, 0);
    field[6] = NULL;

    // field options
    set_field_opts   (field[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts   (field[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_field_opts   (field[2], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts   (field[3], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);
    set_field_opts   (field[4], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts   (field[5], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE);

    // field color
    set_field_back (field[0], COLOR_PAIR(1));
    set_field_back (field[1], COLOR_PAIR(2));
    set_field_back (field[2], COLOR_PAIR(1));
    set_field_back (field[3], COLOR_PAIR(2));
    set_field_back (field[4], COLOR_PAIR(1));
    set_field_back (field[5], COLOR_PAIR(2));

    // field labels
    set_field_buffer (field[0], 0, prompt1);
    set_field_buffer (field[2], 0, prompt2);
    set_field_buffer (field[4], 0, prompt3);

    // form, window, subwindow
    y = 10;
    x = 10;
    form     = new_form (field);
    scale_form   (form, &rows, &cols);    // calculate form subwindow size based on field dimensions
    win_body = newwin (rows+2, cols+2, y, x);    // parent window
    box          (win_body, 0, 0);
    set_form_win (form, win_body);
    set_form_sub (form, derwin (win_body, rows, cols, 1, 1));  // form fields subwindow

    // enable form window key input
    keypad       (form_win (form), TRUE);

    // bottom bar <enter> string
    mvwprintw (form_win (form),
               rows + 1,
               cols - strlen(enter) - 4,
               "%s", enter);

    // render form window
    post_form (form);
    wrefresh  (form_win (form));

    // main loop
    running = true;
    while (running) {

        ch = wgetch (form_win (form));

        switch (ch) {
            // <enter> to sync field data, exit loop
            case '\n':
                form_driver (form, REQ_VALIDATION);
                running = false;
                break;
            case KEY_BACKSPACE:    // backspace
            case 127:
            case '\b':
                form_driver (form, REQ_DEL_PREV);
                break;
            case '\t':
            case KEY_DOWN:         // tab, down -> next field
                form_driver (form, REQ_NEXT_FIELD);
                form_driver (form, REQ_END_LINE);
                break;
            case KEY_BTAB:
            case KEY_UP:           // shift+tab, up -> previous field
                form_driver (form, REQ_PREV_FIELD);
                form_driver (form, REQ_END_LINE);
                break;
            case KEY_LEFT:         // left
                form_driver (form, REQ_PREV_CHAR);
                break;
            case KEY_RIGHT:        // right
                form_driver (form, REQ_NEXT_CHAR);
                break;
            case KEY_DC:           // delete
                form_driver (form, REQ_DEL_CHAR);
                break;
            case KEY_HOME:         // home
                form_driver (form, REQ_BEG_LINE);
                break;
            case KEY_END:          // end
                form_driver (form, REQ_END_LINE);
                break;
            default:               // input character
                form_driver (form, ch);
                break;
        }

        wrefresh (form_win (form));
    }

    // copy field buffer data to buffer, print results
    memcpy    (buffer, trim_whitespaces (field_buffer (field[1], 0)), BUF_LEN - 1);
    mvwprintw (stdscr, 20, 10, "1 >> \"%s\"\n", buffer);

    memcpy    (buffer, trim_whitespaces (field_buffer (field[3], 0)), BUF_LEN - 1);
    mvwprintw (stdscr, 21, 10, "2 >> \"%s\"\n", buffer);

    memcpy    (buffer, trim_whitespaces (field_buffer (field[5], 0)), BUF_LEN - 1);
    mvwprintw (stdscr, 22, 10, "3 >> \"%s\"\n", buffer);
    
    // wait...
    getch();

    unpost_form (form);
    free_form   (form);
    free_field  (field[0]);
    free_field  (field[1]);
    free_field  (field[2]);
    free_field  (field[3]);
    free_field  (field[4]);
    free_field  (field[5]);
    endwin();

    return 0;
}
