
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <libgen.h>

#include "display_lines.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "update_window_data/_update_window_data.h"

#define LINE_NUM_TEXT_SPACES  2

static void  display_lines_buff  (int, int, state_t*);
static void  display_lines_file  (int, window_t*);
static void  format_win_data     (int, state_t*);



void
display_lines (int       type,
               int       key,
               int       plugin_index,
               state_t  *state)
{
    window_t *win = state->plugins[plugin_index]->win;

    if (state->plugins[plugin_index]->has_window) {

        switch (type) {

        case BUFF_TYPE:

            if (win->buff_data->changed) {

                set_buff_rows_cols (win);
                if (plugin_index != Asm) {
                    win->buff_data->scroll_row = 1;
                }
                win->buff_data->scroll_col = 1;
                win->buff_data->changed = false;
            }

            display_lines_buff (key, plugin_index, state);
            break;

        case FILE_TYPE:

            if (win->file_data->path_changed) {

                // free file pointer, offsets
                if (win->file_data->ptr != NULL) {
                    fclose (win->file_data->ptr);
                }
                if (win->file_data->offsets != NULL) {
                    free (win->file_data->offsets);
                }

                // open new file
                win->file_data->ptr = fopen (win->file_data->path, "r");
                if (win->file_data->ptr == NULL) {
                    pfeme ("Unable to open file path \"%s\" for plugin \"%s\".\n",
                            win->file_data->path, win->code);
                }

                set_file_rows_cols (win);

                win->file_data->path_changed = false;
            }

            display_lines_file (key, win);
            break;
        }

        format_win_data (plugin_index, state);
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
display_lines_buff (int      key,
                    int      plugin_index,
                    state_t *state)
{
    char   *buff_ptr,
           *newline_ptr;
    int     wy,
            wx,
            data_row,
            win_rows,
            win_mid_row,
            scroll_row,
            buff_rows,
            len;
    window_t *win;

    win = state->plugins[plugin_index]->win;
    buff_ptr = win->buff_data->buff;
    wy = 0;
    wx = 0;

    wclear (win->DWIN);

    switch (key) {

        case BEG_DATA:
            win->buff_data->scroll_row = 1;
            win->buff_data->scroll_col = 1;
            break;

        case END_DATA:
            if (win->buff_data->rows <= win->data_win_rows) {
                win->buff_data->scroll_row = 1;
            } else {
                win->buff_data->scroll_row = win->buff_data->rows - win->data_win_rows;
            }
            win->buff_data->scroll_col = 1;
            break;

        case ROW_DATA:
            win_rows = win->data_win_rows;
            win_mid_row = win->data_win_rows / 2;
            buff_rows = win->buff_data->rows;
            scroll_row = win->buff_data->scroll_row;
                //
            win->buff_data->scroll_col = 1;
                //
            if (scroll_row <= win_mid_row) {
                win->buff_data->scroll_row = 1;
            } else if (scroll_row >= (buff_rows - win_mid_row)) {
                win->buff_data->scroll_row = buff_rows - win_rows + 1;
            } else {
                win->buff_data->scroll_row = scroll_row - win_mid_row;
            }
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
    while (data_row < win->buff_data->scroll_row) {
        if (*buff_ptr++ == '\n') {
            ++data_row;
        }
    }

    // print buffer lines
    while ((newline_ptr = strchr(buff_ptr, '\n')) != NULL && wy < win->data_win_rows) {

        len = newline_ptr - buff_ptr;

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

        len = strlen (buff_ptr);

        if (len > (win->buff_data->scroll_col - 1)) {

            if (len - win->buff_data->scroll_col > win->data_win_cols) {
                len = win->data_win_cols;
            } else {
                len = len - win->buff_data->scroll_col + 1;
            }

            mvwprintw(win->DWIN, wy, wx, "%.*s", len, buff_ptr + win->buff_data->scroll_col - 1);
        } 

        else {
            mvwprintw(win->DWIN, wy, wx, " ");
        }
    }

    wrefresh (win->DWIN);
}
                


/*
    Calculate file rows, columns, etc.
*/
void
set_file_rows_cols (window_t *win)
{
    char  line [512];
    int   line_len,
          max_mid,
          ch, n;

    win->file_data->first_char = 0;
    win->file_data->rows = 0;
    win->file_data->max_cols = 0;

    // calculate rows, max columns
    while (fgets(line, sizeof(line), win->file_data->ptr) != NULL) {
        line_len = strlen(line);
        if (win->file_data->max_cols < line_len) {
            win->file_data->max_cols = line_len + 1;
        }
        win->file_data->rows += 1;
    }
    rewind (win->file_data->ptr);

    // calculate max line number digits
    n = win->file_data->rows;
    win->file_data->line_num_digits = 0;
    while (n != 0) {
        ++win->file_data->line_num_digits;
        n /= 10;
    }

    // calculate newline offsets
    win->file_data->offsets = (long*) malloc (win->file_data->rows * sizeof(long));
    if (win->file_data->offsets == NULL) {
        pfeme ("Failed to allocate offsets array for \"%s\" (%s)\n", win->file_data->path, win->code);
    }
    win->file_data->offsets [0] = 0;
    for (int i = 1; i < win->file_data->rows; i++) {
        while ((ch = fgetc (win->file_data->ptr)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            win->file_data->offsets[i] = ftell (win->file_data->ptr);
    }
    rewind (win->file_data->ptr);

    win->file_data->min_mid = (win->data_win_rows / 2) + 1;
    max_mid = win->file_data->rows - ((win->data_win_rows - 1) / 2);
    win->file_data->max_mid = (win->file_data->rows > win->data_win_rows) ? max_mid : win->file_data->min_mid;
    win->data_win_mid_line = win->file_data->min_mid;
}



static void
display_lines_file (int key,
                    window_t *win) 
{
    char line[256];
    char buff[16];
    int  row = 0,
         col = 0,
         print_line,
         line_len,
         line_index,
         i, spaces, win_text_len;

    wclear (win->DWIN);

    // shift mid_line, first_char
    // TODO: page up/down, home, end
    switch (key) {

        case BEG_DATA:
            break;

        case LINE_DATA:
            if (win->file_data->line < win->file_data->min_mid) {
                win->data_win_mid_line = win->file_data->min_mid;
            } else if (win->file_data->line > win->file_data->max_mid) {
                win->data_win_mid_line = win->file_data->max_mid;
            } else {
                win->data_win_mid_line = win->file_data->line;
            }
            break;

        case KEY_UP:
            win->data_win_mid_line =
                (win->data_win_mid_line <= win->file_data->min_mid)
                ? win->file_data->min_mid
                : win->data_win_mid_line - 1;
            break;

        case KEY_DOWN:
            win->data_win_mid_line =
                (win->data_win_mid_line >= win->file_data->max_mid)
                ? win->file_data->max_mid
                : win->data_win_mid_line + 1;
            break;

        case KEY_RIGHT:
            win->file_data->first_char =
                ((win->file_data->max_cols - win->file_data->first_char) >
                    (win->data_win_cols - win->file_data->line_num_digits - LINE_NUM_TEXT_SPACES))
                ? win->file_data->first_char + 1
                : win->file_data->max_cols - win->data_win_cols + win->file_data->line_num_digits + LINE_NUM_TEXT_SPACES;
            break;

        case KEY_LEFT:
            win->file_data->first_char =
                (win->file_data->first_char == 0)
                ? 0
                : win->file_data->first_char - 1;
            break;
    }

    // calculate first line
    print_line = win->data_win_mid_line - (win->data_win_rows / 2);
    print_line = (print_line >= 1) ? print_line : 1;

    // calculate text length minus line number, spaces
    win_text_len = win->data_win_cols - win->file_data->line_num_digits - LINE_NUM_TEXT_SPACES;

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
            if (line [line_len - 1] == '\n') {
                line_len -= 1;
            } 

            // calculate line length
            else {
                line_len = ((line_len - win->file_data->first_char) <= win->data_win_cols) 
                            ? line_len - win->file_data->first_char
                            : win->data_win_cols;
            }

            line_index = win->file_data->first_char;

        } else {

            line [0] = ' ';
            line [1] = '\0';
            line_len = 1;
            line_index = 0;
        }

        spaces = win->file_data->line_num_digits - sprintf (buff, "%d", print_line - 1) + LINE_NUM_TEXT_SPACES;

        // highlight current line
        if ((print_line - 1) == win->file_data->line) {
            wattron (win->DWIN, A_REVERSE);
        }

        // print line
        mvwprintw (win->DWIN, row++, col, "%d%*c%.*s",
                print_line - 1, spaces, ' ', win_text_len, (const char*)(line + line_index));

        if ((print_line - 1) == win->file_data->line) {
            wattroff (win->DWIN, A_REVERSE);
        }

        // break if end of file
        if (print_line > win->file_data->rows) {
            break;
        }
    }

    wrefresh(win->DWIN);
}



static void
format_win_data (int plugin_index,
                 state_t *state)
{
    int       i, j, 
              m, n,
              ch,
              rows, cols,
              left_spaces,
              right_spaces;
    size_t    k, si;
    window_t *win;
    char     *needle,
             *basefile;

    win = state->plugins[plugin_index]->win;

    if (plugin_index == Asm) {

        // highlight current hex address (wherever it occurs)
        si = 0;
        getmaxyx (win->DWIN, rows, cols);
        needle = state->plugins[Src]->win->file_data->addr;

        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {
                ch = mvwinch (win->DWIN, i, j);
                if ((char) ch == needle [si]) {
                    if (si == 0) {
                        m = i;
                        n = j;
                    }
                    si += 1;
                    if (si == strlen (needle)) {
                        wattron (win->DWIN, A_REVERSE);
                        for (k = 0; k < strlen (needle); k++) {
                            mvwprintw (win->DWIN, m, n + k, "%c", needle[k]);
                        }
                        wattroff (win->DWIN, A_REVERSE);
                        wrefresh (win->DWIN);
                    }
                } 
                else {
                    si = 0;
                }
            }
        }
    }

    else if (plugin_index == Src) {
            
        // TODO: Replace source file line number with colored "b2" breakpoint index number

        // print current source code file in top bar
        basefile = basename (win->file_data->path);
        left_spaces = (win->topbar_cols - strlen (basefile)) / 2;
        right_spaces = win->topbar_cols - strlen (basefile) - left_spaces;
        left_spaces = left_spaces > 0 ? left_spaces : 0;
        right_spaces = right_spaces > 0 ? left_spaces : 0;
            //
        wattron   (win->IWIN, COLOR_PAIR(WINDOW_INPUT_TITLE_COLOR));
        mvwprintw (win->IWIN, 0, 0, "%*c%.*s%*c", left_spaces, ' ',
                                                  win->topbar_cols, basefile,
                                                  right_spaces, ' ');
        wattroff  (win->IWIN, COLOR_PAIR(WINDOW_INPUT_TITLE_COLOR));
        wrefresh  (win->IWIN);
    }
}

