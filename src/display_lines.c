
#include <string.h>
#include <ncurses.h>

#include "display_lines.h"
#include "update_window_data/_update_window_data.h"


static void  display_lines_buff (int key, window_t *win);
static void  display_lines_file (int key, window_t *win);



void
display_lines (int type,
               int key,
               window_t *win)
{
    switch (type) {
        case BUFF_TYPE:
            display_lines_buff (key, win);
            break;
        case FILE_TYPE:
            display_lines_file (key, win);
            break;
    }
}



void
set_buff_rows_cols (window_t *win)
{
    char *buff_ptr = win->buff_data->buff;
    int   curr_cols = 0;

    win->buff_data->rows = 1;
    win->buff_data->max_cols = 0;

    while (*buff_ptr != '\0') {
        if (*buff_ptr == '\n') {
            ++win->buff_data->rows;
            if (curr_cols > win->buff_data->max_cols) {
                win->buff_data->max_cols = curr_cols;
            }
            curr_cols = 0;
        } else {
            ++curr_cols;
        }
        ++buff_ptr;
    }
}



static void
display_lines_buff (int key,
                    window_t *win)
{
    char   *buff_ptr = win->buff_data->buff,
           *newline_ptr;
    int     wy = 0,
            wx = 0,
            data_row;

    wclear (win->DWIN);

    switch (key) {
        case NEW_WIN:
            break;
        case KEY_UP:
            if (win->buff_data->scroll_row > 1) {
                --win->buff_data->scroll_row;
            }
            break;
        case KEY_DOWN:
            if ((win->buff_data->rows - win->buff_data->scroll_row) >= win->data_win_rows) {
                ++win->buff_data->scroll_row;
            }
            break;
        case KEY_LEFT:
            if (win->buff_data->scroll_col > 1) {
                --win->buff_data->scroll_col;
            }
            break;
        case KEY_RIGHT:
            if ((win->buff_data->max_cols - win->buff_data->scroll_col) >= win->data_win_cols) {
                ++win->buff_data->scroll_col;
            }
            break;
    }

    // move buffer pointer to beginning of current row
    data_row = 1;
    while (data_row != win->buff_data->scroll_row) {
        if (*buff_ptr++ == '\n') {
            ++data_row;
        }
    }

    // print buffer lines
    while ((newline_ptr = strchr(buff_ptr, '\n')) != NULL && wy < win->data_win_rows) {

        int len = newline_ptr - buff_ptr;

        if (len >= win->buff_data->scroll_col) {
            if (len - win->buff_data->scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - (win->buff_data->scroll_col - 1);
            }
            mvwprintw (win->DWIN, wy, wx, "%.*s", len, buff_ptr + (win->buff_data->scroll_col - 1));
        } else {
            mvwprintw (win->DWIN, wy, wx, " ");
        }

        buff_ptr = newline_ptr + 1;
        wx = 0;
        ++wy;
    }

    // print last line if needed
    if (*buff_ptr != '\0' && wy <= win->data_win_rows) {
        int len = strlen (buff_ptr);

        if (len > win->buff_data->scroll_col) {
            if (len - win->buff_data->scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - win->buff_data->scroll_col;
            }
            mvwprintw(win->DWIN, wy, wx, "%.*s", len, buff_ptr + win->buff_data->scroll_col);
        } else {
            mvwprintw(win->DWIN, wy, wx, " ");
        }
    }

    wrefresh (win->DWIN);
}
                


/*
    Display file lines
*/

void
get_file_rows_cols (window_t *win) {
    win->file_data->rows = 0;
    win->file_data->max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), win->file_data->ptr) != NULL) {
        line_len = strlen(line);
        if (win->file_data->max_cols < line_len) {
            win->file_data->max_cols = line_len + 1;
        }
        win->file_data->rows += 1;
    }
    rewind (win->file_data->ptr);
}


static void
display_lines_file (int key,
                    window_t *win) 
{
    char line[256];
    int  row = 0,
         col = 0,
         print_line,
         line_len,
         line_index,
         i;

    wclear (win->DWIN);

    // shift mid_line, first_char
    switch (key) {
        case 0:
            break;
        case KEY_UP:
            win->data_win_mid_line = (win->data_win_mid_line <= win->file_data->min_mid) ? win->file_data->min_mid : win->data_win_mid_line - 1;
            break;
        case KEY_DOWN:
            win->data_win_mid_line = (win->data_win_mid_line >= win->file_data->max_mid) ? win->file_data->max_mid : win->data_win_mid_line + 1;
            break;
        case KEY_RIGHT:
            win->file_data->first_char = ((win->file_data->max_cols - win->file_data->first_char) > win->data_win_cols)
                ? win->file_data->first_char + 1
                : win->file_data->max_cols - win->data_win_cols;
            break;
        case KEY_LEFT:
            win->file_data->first_char = (win->file_data->first_char == 0) ? 0 : win->file_data->first_char - 1;
            break;
    }

    // calculate first line
    print_line = win->data_win_mid_line - (win->data_win_rows / 2);

    // print lines
    for (i = 0; i < win->data_win_rows; i++) {

        // seek to beginning of line
        fseek (win->file_data->ptr, win->file_data->offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), win->file_data->ptr);
        line_len = strlen (line);

        // if line characters visible
        if (win->file_data->first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n')
                line_len -= 1;

            // calculate line length
            line_len = ((line_len - win->file_data->first_char) <= win->data_win_cols) 
                        ? line_len - win->file_data->first_char
                        : win->data_win_cols;

            // set line start index
            line_index = win->file_data->first_char;
        } 

        // if no characters visible
        else {
            line [0] = ' ';
            line_len = 1;
            line_index = 0;
        }

        mvwaddnstr (win->DWIN, row++, col, (const char*)(line + line_index), line_len);

        // break if end of file
        if (print_line > win->file_data->rows) {
            break;
        }
    }

    wrefresh(win->DWIN);
}
