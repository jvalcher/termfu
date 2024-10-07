
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <errno.h>

#include "display_lines.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "format_window_data.h"

#define LINE_NUM_TEXT_SPACES  2

static void display_lines_buff_no_wrap  (int, int, state_t*);
static void display_lines_buff_wrap     (int, int, state_t*);
static void display_lines_file          (int, window_t*);



int
display_lines (int       type,
               int       key,
               int       plugin_index,
               state_t  *state)
{
    int ret;
    window_t *win = state->plugins[plugin_index]->win;

    if (state->plugins[plugin_index]->has_window) {

        switch (type) {

        case BUFF_TYPE:

            if (win->buff_data->changed) {

                // text not wrapped
                if (win->buff_data->text_wrapped == false) {

                    // configure buffer data
                    set_buff_rows_cols (win);
                    if (plugin_index != Asm) {
                        win->buff_data->scroll_row = 1;
                    }
                    win->buff_data->scroll_col = 1;
                    win->buff_data->changed = false;

                    display_lines_buff_no_wrap (key, plugin_index, state);
                }

                // text wrapped
                else {
                    display_lines_buff_wrap (key, plugin_index, state);
                }
            }

            break;

        case FILE_TYPE:

            if (win->src_file_data->path_changed) {

                // free file pointer, offsets
                if (win->src_file_data->ptr != NULL) {
                    fclose (win->src_file_data->ptr);
                }
                if (win->src_file_data->offsets != NULL) {
                    free (win->src_file_data->offsets);
                }

                // open new file
                if ((win->src_file_data->ptr = fopen (win->src_file_data->path, "r")) == NULL) {
                    pfem ("fopen error: %s", strerror (errno));
                    pem ("Failed to open file \"%s\"", win->src_file_data->path);
                    goto disp_lines_err;
                }

                // configure file data
                ret = set_file_rows_cols (win);
                if (ret == FAIL) {
                    pfem ("Failed to set file rows, cols");
                    goto disp_lines_err;
                }
                win->src_file_data->path_changed = false;
            }

            // display lines
            if (win->src_file_data->ptr != NULL) {
                display_lines_file (key, win);
            } else {
                pfem ("NULL file pointer");
                goto disp_lines_err;
            }

            break;
        }

        format_window_data (plugin_index, state);

    }

    return A_OK;

disp_lines_err:

    pemr ("Key: \"%d\", plugin index: \"%d\", plugin code: \"%s\"",
                key, plugin_index, get_plugin_code (plugin_index));
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
display_lines_buff_no_wrap (int      key,
                            int      plugin_index,
                            state_t *state)
{
    char     *buff_ptr,
             *newline_ptr;
    int       wy,
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
    while ((newline_ptr = strchr (buff_ptr, '\n')) != NULL && wy < win->data_win_rows) {

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



// CURRENT: scroll not working

static void
display_lines_buff_wrap (int      key,
                         int      plugin_index,
                         state_t *state)
{
    int       y, x, c,
              rows, cols,
              offset;
    char     *buff_ptr;
    window_t *win;

    win = state->plugins[plugin_index]->win;
    getyx (win->DWIN, y, x);
    (void) x;
    rows = win->rows;
    cols = win->cols;

    switch (key) {

        case END_DATA:
        case ROW_DATA:
        case BEG_DATA:
        case KEY_HOME:
            win->buff_data->buff_ptr = win->buff_data->buff;
            break;

        case KEY_DOWN:
            c = 0;
            buff_ptr = win->buff_data->buff_ptr;
            if (y >= (rows - 1)) {
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

        case KEY_UP:
            c = 0;
            buff_ptr = win->buff_data->buff_ptr;
            if (buff_ptr != win->buff_data->buff) {
                if (*(buff_ptr - 1) == '\n') {
                    if ((buff_ptr - 1) != win->buff_data->buff) {
                        ++c;
                        --buff_ptr;
                        offset = 2;
                    }
                }
                else {
                    offset = 1;
                }
                while (*(buff_ptr - c) != '\n' &&
                        (buff_ptr - c) != win->buff_data->buff) {
                    ++c;
                }
                        
                if ((buff_ptr - 1) == win->buff_data->buff) {
                    buff_ptr = win->buff_data->buff;
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

        default:
            win->buff_data->buff_ptr = win->buff_data->buff;
    }

    wmove     (win->DWIN, 0, 0); 
    waddstr   (win->DWIN, win->buff_data->buff_ptr);
    wclrtoeol (win->DWIN);
    wrefresh  (win->DWIN);
}
                


/*
    Calculate file rows, columns, etc.
*/
int
set_file_rows_cols (window_t *win)
{
    char  line [512];
    int   line_len,
          max_mid,
          ch, n;

    win->src_file_data->first_char = 0;
    win->src_file_data->rows = 0;
    win->src_file_data->max_cols = 0;

    // calculate rows, max columns
    while (fgets(line, sizeof(line), win->src_file_data->ptr) != NULL) {
        line_len = strlen(line);
        if (win->src_file_data->max_cols < line_len) {
            win->src_file_data->max_cols = line_len + 1;
        }
        win->src_file_data->rows += 1;
    }
    rewind (win->src_file_data->ptr);

    // calculate max line number digits
    n = win->src_file_data->rows;
    win->src_file_data->line_num_digits = 0;
    while (n != 0) {
        ++win->src_file_data->line_num_digits;
        n /= 10;
    }

    // calculate newline offsets
    if ((win->src_file_data->offsets = (long*) malloc (win->src_file_data->rows * sizeof(long))) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Failed to allocate offsets array for \"%s\" (%s)\n", win->src_file_data->path, win->code);
    }
    win->src_file_data->offsets [0] = 0;
    for (int i = 1; i < win->src_file_data->rows; i++) {
        while ((ch = fgetc (win->src_file_data->ptr)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            win->src_file_data->offsets[i] = ftell (win->src_file_data->ptr);
    }
    rewind (win->src_file_data->ptr);

    win->src_file_data->min_mid = (win->data_win_rows / 2) + 1;
    max_mid = win->src_file_data->rows - ((win->data_win_rows - 1) / 2);
    win->src_file_data->max_mid = (win->src_file_data->rows > win->data_win_rows) ? max_mid : win->src_file_data->min_mid;
    win->data_win_mid_line = win->src_file_data->min_mid;

    return A_OK;
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

    // TODO: page up/down, home, end
    switch (key) {

        case BEG_DATA:
            break;

        case LINE_DATA:
            if (win->src_file_data->line < win->src_file_data->min_mid) {
                win->data_win_mid_line = win->src_file_data->min_mid;
            } else if (win->src_file_data->line > win->src_file_data->max_mid) {
                win->data_win_mid_line = win->src_file_data->max_mid;
            } else {
                win->data_win_mid_line = win->src_file_data->line;
            }
            break;

        case KEY_UP:
            win->data_win_mid_line =
                (win->data_win_mid_line <= win->src_file_data->min_mid)
                ? win->src_file_data->min_mid
                : win->data_win_mid_line - 1;
            break;

        case KEY_DOWN:
            win->data_win_mid_line =
                (win->data_win_mid_line >= win->src_file_data->max_mid)
                ? win->src_file_data->max_mid
                : win->data_win_mid_line + 1;
            break;

        case KEY_RIGHT:
            win->src_file_data->first_char =
                ((win->src_file_data->max_cols - win->src_file_data->first_char) >
                    (win->data_win_cols - win->src_file_data->line_num_digits - LINE_NUM_TEXT_SPACES))
                ? win->src_file_data->first_char + 1
                : win->src_file_data->max_cols - win->data_win_cols + win->src_file_data->line_num_digits + LINE_NUM_TEXT_SPACES;
            break;

        case KEY_LEFT:
            win->src_file_data->first_char =
                (win->src_file_data->first_char == 0)
                ? 0
                : win->src_file_data->first_char - 1;
            break;
    }

    // calculate first line
    print_line = win->data_win_mid_line - (win->data_win_rows / 2);
    print_line = (print_line >= 1) ? print_line : 1;

    // calculate text length minus line number, spaces
    win_text_len = win->data_win_cols - win->src_file_data->line_num_digits - LINE_NUM_TEXT_SPACES;

    // print lines
    for (i = 0; i < win->data_win_rows; i++) {

        // seek to beginning of line
        fseek (win->src_file_data->ptr, win->src_file_data->offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), win->src_file_data->ptr);
        line_len = strlen (line);

        // if line characters visible
        if (win->src_file_data->first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n') {
                line_len -= 1;
            } 

            // calculate line length
            else {
                line_len = ((line_len - win->src_file_data->first_char) <= win->data_win_cols) 
                            ? line_len - win->src_file_data->first_char
                            : win->data_win_cols;
            }

            line_index = win->src_file_data->first_char;

        } else {

            line [0] = ' ';
            line [1] = '\0';
            line_len = 1;
            line_index = 0;
        }

        spaces = win->src_file_data->line_num_digits - sprintf (buff, "%d", print_line - 1) + LINE_NUM_TEXT_SPACES;

        // print line
        mvwprintw (win->DWIN, row++, col, "%d%*c%.*s",
                print_line - 1, spaces, ' ', win_text_len, (const char*)(line + line_index));

        // break if end of file
        if (print_line > win->src_file_data->rows) {
            break;
        }
    }

    wrefresh(win->DWIN);
}

