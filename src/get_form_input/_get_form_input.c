#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <ncurses.h>
#include <form.h>

#include "../data.h"
#include "../utilities.h"

char* trim_whitespaces (char *str);



int
get_form_input (char *prompt,
                char *buffer)
{
    int     rows, cols,
            input_rows, label_rows,
            total_rows, total_cols,
            scr_rows, scr_cols,
            y, x,
            ch;
    bool    running,
            form_closed;
    FORM   *form;
    FIELD  *field[4];
    WINDOW *win_body,
           *curr_layout;

    char *enter = " Enter ",
         *esc   = " Esc ";

    curs_set (1);
    echo ();

    // save current layout
    doupdate ();
    if ((curr_layout = dupwin (curscr)) == NULL) {
        pfemr ("Failed to duplicate current screen");
    }

    // fields
    cols = 64;
    label_rows = 2;
    input_rows = 2;
    field[0] = new_field (label_rows, cols, 0, 0, 0, 0);
    field[1] = new_field (input_rows, cols, 2, 0, 8, 0);    // 8 extra rows
    field[2] = NULL;
    set_field_opts   (field[0], O_VISIBLE | O_PUBLIC | O_AUTOSKIP);
    set_field_opts   (field[1], O_VISIBLE | O_PUBLIC | O_EDIT | O_ACTIVE );
    set_field_back   (field[1], COLOR_PAIR(FORM_INPUT_FIELD));
    set_field_buffer (field[0], 0, prompt);

    // form position
    total_rows = label_rows + input_rows + 4;
    total_cols = cols + 4;
    getmaxyx (stdscr, scr_rows, scr_cols);
    cols = (scr_cols > 60) ? 64 : (scr_cols - 2);
    y = (scr_rows > total_rows) ? ((scr_rows - total_rows) / 2) : 1;
    x = (scr_cols > total_cols) ? ((scr_cols - total_cols) / 2) : 1;

    // form window
    form     = new_form (field);
    scale_form   (form, &rows, &cols);
    win_body = newwin (rows + 4, cols+4, y, x);
    box          (win_body, 0, 0);
    set_form_win (form, win_body);
    set_form_sub (form, derwin (win_body, rows, cols, 2, 2));

    // bottom border command strings
    wattron (form_win (form), COLOR_PAIR(FORM_BUTTON));
    mvwprintw (form_win (form), 0, cols - strlen(esc) - 2, "%s", esc);
    cols = (cols - strlen (enter)) / 2;
    mvwprintw (form_win (form), rows + 3, cols, "%s", enter);
    wattroff (form_win (form), COLOR_PAIR(FORM_BUTTON));

    keypad       (form_win (form), TRUE);
    post_form (form);
    wrefresh  (form_win (form));

    running = true;
    form_closed = false;
    while (running) {

        ch = wgetch (form_win (form));

        switch (ch) {
            case '\n':              // enter, sync buffer(s), quit
                form_driver (form, REQ_VALIDATION);
                running = false;
                break;
            case ESC:              // escape
                form_closed = true;
                running = false;
                break;
            case KEY_BACKSPACE:    // backspace
            case 127:
            case '\b':
                form_driver (form, REQ_DEL_PREV);
                break;
            case KEY_DOWN:          // scroll down
                form_driver (form, REQ_SCR_FLINE);
                break;
            case KEY_UP:            // scroll up
                form_driver (form, REQ_SCR_BLINE);
                break;
            case '\t':              // next field
                form_driver (form, REQ_NEXT_FIELD);
                form_driver (form, REQ_END_LINE);
                break;
            case KEY_BTAB:          // previous field
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

    if (form_closed) {

        // no input
        buffer [0] = '\0';

    } else {
        
        // copy field buffer to input buffer
        memcpy (buffer, trim_whitespaces (field_buffer (field[1], 0)), INPUT_BUFF_LEN - 1);
        buffer [INPUT_BUFF_LEN - 1] = '\0';
    }

    // close form
    unpost_form (form);
    free_form   (form);
    free_field  (field[0]);
    free_field  (field[1]);
    free_field  (field[2]);

    // restore previous layout
    touchwin (curr_layout);
    wnoutrefresh (curr_layout);
    doupdate ();
    curs_set (0);
    noecho ();

    return A_OK;
}



char*
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

