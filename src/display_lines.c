
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "display_lines.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "format_window_data.h"

#define LINE_NUM_TEXT_SPACES  2



int
display_lines (int      key,
               int      plugin_index,
               state_t *state)
{
    int ret;
    window_t *win = state->plugins[plugin_index]->win;

    if (state->plugins[plugin_index]->has_window) {

        if (win->buff_data->changed) {

            ret = create_scroll_buffer_llist (plugin_index, state);
            if (ret == FAIL) {
                pfem ("Failed to create scroll_buff_line_t linked list");
                goto disp_lines_err;
            }

            win->buff_data->changed = false;
        }

        ret = display_scroll_buff_lines (key, plugin_index, state);
        if (ret == FAIL) {
            pfem ("Failed to display window lines");
            goto disp_lines_err;
        }

        ret = format_window_data (plugin_index, state);
        if (ret == FAIL) {
            pfem ("Failed to format window data");
            goto disp_lines_err;
        }
    }

    return A_OK;

disp_lines_err:

    pemr ("Key: \"%d\", plugin index: \"%d\", plugin code: \"%s\"",
                key, plugin_index, get_plugin_code (plugin_index));
}



int
create_scroll_buffer_llist (int      plugin_index,
                            state_t *state)
{
    int   chars,
          max_chars;
    char *ptr;
    window_t           *win;
    buff_data_t        *buff_data;
    scroll_buff_line_t *buff_line,
                       *curr_buff_line,
                       *tmp_buff_line;

    // free scroll_buff_line_t linked list
    win = state->plugins[plugin_index]->win;
    buff_data = win->buff_data;
    buff_line = buff_data->head_line;
    while (buff_line != NULL) {
        tmp_buff_line = buff_line->next;
        free (buff_line); 
        buff_line = tmp_buff_line;
    }

    buff_data->head_line = NULL;
    buff_data->tail_line = NULL;
    buff_data->curr_line = NULL;
    curr_buff_line       = NULL;
    buff_data->rows      = 0;

    // update source file buffer if path changed
    if (plugin_index == Src && state->debugger->path_changed) {

        if (buff_data->buff != NULL) {
            free (buff_data->buff);
        }

        buff_data->buff = create_buff_from_file (state->debugger->path_buffer);
        if (buff_data->buff == NULL) {
            pfemr ("Failed to create buffer from file \"%s\"", state->debugger->path_buffer);
        }

        state->debugger->path_changed = false;
    }

    ptr = buff_data->buff;

    // create scroll_buff_line_t linked list
    do {

        // allocate scroll_buff_line_t
        if ((buff_line = (scroll_buff_line_t*) malloc (sizeof (scroll_buff_line_t))) == NULL) {
            pfem ("malloc error: \"%s\"", strerror (errno));
            pemr ("Failed to allocate scroll_buff_line_t");
        }
        if (buff_data->head_line == NULL) {
            buff_data->head_line = buff_line;
            curr_buff_line = buff_data->head_line;
            curr_buff_line->prev = NULL;
            buff_data->head_line->line = 1;
        } else {
            buff_line->line = curr_buff_line->line + 1;
            buff_line->prev = curr_buff_line;
            curr_buff_line->next = buff_line;
            curr_buff_line = buff_line;
        }
        curr_buff_line->next = NULL;
        buff_data->rows += 1;

        // add text pointer, number of characters
        chars = 1;
        curr_buff_line->ptr = ptr;
        while (*ptr != '\n' && *ptr != '\0') {
            if (plugin_index != Asm && plugin_index != Src) {
                if (chars >= win->data_win_cols) {
                    break;
                }
            }
            ++chars;
            ++ptr;
        }

        curr_buff_line->len = chars;

        if (chars > max_chars) {
            max_chars = chars;
        }
        if (*ptr == '\n') {
            ++ptr;
        }
        
    } while (*ptr != '\0');

    buff_data->scroll_row = 1;
    buff_data->scroll_col_offset = 0;
    buff_data->max_chars = max_chars;
    buff_data->tail_line = curr_buff_line;
    buff_data->rows = curr_buff_line->line;

    return A_OK;
}



int
display_scroll_buff_lines (int      key,
                           int      plugin_index,
                           state_t *state)
{
    int   i,
          rem_lines,
          text_len,
          print_len,
          max_scroll_row,
          print_row,
          line_num_len,
          last_line_num,
          rem_spaces;
    char *ptr,
         *blank_line = " ";
    window_t           *win;
    buff_data_t        *buff_data;
    scroll_buff_line_t *buff_line;

    win       = state->plugins[plugin_index]->win;
    buff_data = win->buff_data;

    werase (win->DWIN);

    if (buff_data->curr_line == NULL) {
        buff_data->curr_line = buff_data->head_line;
    }

    switch (key) {

        case BEG_DATA:
        case KEY_HOME:
            buff_data->scroll_row = 1;
            buff_data->curr_line = buff_data->head_line;
            break;

        case END_DATA:
        case KEY_END:
            if (buff_data->rows < win->data_win_rows) {
                buff_data->scroll_row = 1;
                buff_data->curr_line = buff_data->head_line;
            } else {
                buff_data->curr_line = buff_data->tail_line;
                buff_data->scroll_row = buff_data->rows;
                for (i = 0; i < (win->data_win_rows - 1); i++) {
                    buff_data->curr_line = buff_data->curr_line->prev;
                    --buff_data->scroll_row;
                }
            }
            break;

        case ROW_DATA:

            if (buff_data->rows <= win->data_win_rows) {
                print_row = 1;
            } else {
                switch (plugin_index) {
                    case Src:
                        print_row = state->debugger->curr_Src_line - (win->data_win_rows / 2);
                        break;
                    case Asm:
                        print_row = state->debugger->curr_Asm_line - (win->data_win_rows / 2);
                        break;
                    default:
                        print_row = 1;
                }
                if (print_row < 1) {
                    print_row = 1;
                } else if (print_row > (buff_data->tail_line->line - win->data_win_rows + 1)) {
                    print_row = buff_data->tail_line->line - win->data_win_rows + 1;
                } 
            }
            buff_data->scroll_row = print_row;

            if (buff_data->curr_line->line < print_row) {
                while (buff_data->curr_line->line != print_row) {
                    buff_data->curr_line = buff_data->curr_line->next;
                }
            } else if (buff_data->curr_line->line > print_row) {
                while (buff_data->curr_line->line != print_row) {
                    buff_data->curr_line = buff_data->curr_line->prev;
                }
            }
            break;

        case KEY_PPAGE:
            if ((buff_data->scroll_row - win->data_win_rows) <= 0) {
                buff_data->scroll_row = 1;
                buff_data->curr_line = buff_data->head_line;
            } else {
                buff_data->scroll_row -= (win->data_win_rows - 1);
                for (i = 0; i < (win->data_win_rows - 1); i++) {
                    buff_data->curr_line = buff_data->curr_line->prev;
                    --buff_data->scroll_row;
                }
            }
            break;

        case KEY_NPAGE:
            rem_lines = buff_data->rows - buff_data->scroll_row;
            max_scroll_row = buff_data->rows - (win->data_win_rows - 1);
            if (rem_lines >= win->data_win_rows) {
                for (i = 0; i < (win->data_win_rows - 1); i++) {
                    if (buff_data->scroll_row == max_scroll_row) {
                        break;
                    }
                    buff_data->curr_line = buff_data->curr_line->next;
                    ++buff_data->scroll_row;
                } 
            } else {
                while (buff_data->scroll_row < max_scroll_row) {
                    buff_data->curr_line = buff_data->curr_line->next;
                    ++buff_data->scroll_row;
                }
            }
            break;

        case 'j':
        case KEY_DOWN:
            rem_lines = buff_data->rows - buff_data->scroll_row;
            if (rem_lines >= win->data_win_rows) {
                buff_data->curr_line = buff_data->curr_line->next;
                ++buff_data->scroll_row;
            }
            break;

        case 'k':
        case KEY_UP:
            if (buff_data->scroll_row > 1) {
                buff_data->curr_line = buff_data->curr_line->prev;
                --buff_data->scroll_row;
            }
            break;

        case 'h':
        case KEY_LEFT:
            if (buff_data->scroll_col_offset > 0) {
                --buff_data->scroll_col_offset;
            }
            break;

        case 'l':
        case KEY_RIGHT:
            if (buff_data->max_chars - buff_data->scroll_col_offset > win->data_win_cols) {
                ++buff_data->scroll_col_offset;
            }
            break;

    }
    
    // calculate line number offset (Src)
    if (win->index == Src) {
        line_num_len = 1;
        last_line_num = win->buff_data->tail_line->line;
        while (last_line_num > 0) {
            ++line_num_len;
            last_line_num /= 10;
        }
    } else {
        line_num_len = 0;
    }

    // print lines
    i = 0;
    buff_line = buff_data->curr_line;
    text_len = win->data_win_cols - line_num_len;

    while (i < win->data_win_rows && buff_line != NULL) {

        switch (win->index) {

            case Asm:

                if ((buff_line->len - buff_data->scroll_col_offset) < win->data_win_cols) {
                    print_len = buff_line->len - buff_data->scroll_col_offset;
                    if (print_len <= 0) {
                        ptr = blank_line; 
                    } else {
                        ptr = buff_line->ptr + buff_data->scroll_col_offset;
                    }
                } else {
                    print_len = win->data_win_cols;
                    ptr = buff_line->ptr + buff_data->scroll_col_offset;
                }
                waddnstr (win->DWIN, ptr, print_len);

                break;

            case Src:

                // line number
                if      (buff_line->line < 10) rem_spaces = line_num_len - 1;
                else if (buff_line->line < 100) rem_spaces = line_num_len - 2;
                else if (buff_line->line < 1000) rem_spaces = line_num_len - 3;
                else if (buff_line->line < 10000) rem_spaces = line_num_len - 4;
                else if (buff_line->line < 100000) rem_spaces = line_num_len - 5;
                else if (buff_line->line < 1000000) rem_spaces = line_num_len - 6;
                else 
                {
                    pfemr ("Max buffer line length exceeded (last line num: %d, len: %d)",
                                last_line_num, line_num_len - 1);
                }
                wattron  (win->DWIN, COLOR_PAIR(SRC_LINE_COLOR));
                wprintw  (win->DWIN, "%d%*c", buff_line->line, rem_spaces, ' ');
                wattroff (win->DWIN, COLOR_PAIR(SRC_LINE_COLOR));

                // text
                if ((buff_line->len - buff_data->scroll_col_offset) < win->data_win_cols) {
                    print_len = buff_line->len - buff_data->scroll_col_offset;
                    if (print_len <= 0) {
                        ptr = blank_line; 
                    } else {
                        ptr = buff_line->ptr + buff_data->scroll_col_offset;
                    }
                } else {
                    print_len = text_len;
                    ptr = buff_line->ptr + buff_data->scroll_col_offset;
                }
                waddnstr (win->DWIN, ptr, print_len);

                break;

            default:
                waddnstr (win->DWIN, buff_line->ptr, buff_line->len);
                break;
        }

        buff_line = buff_line->next;
        ++i;
    }

    wrefresh (win->DWIN);

    return A_OK;
}



/*

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
    int       x, c,
              rows, cols,
              offset;
    char     *buff_ptr;
    window_t *win;

    win = state->plugins[plugin_index]->win;
    (void) x;
    rows = win->data_win_rows;
    cols = win->data_win_cols;

    switch (key) {

        case END_DATA:
        case ROW_DATA:
        case BEG_DATA:
        case KEY_HOME:
            win->buff_data->buff_ptr = win->buff_data->buff;
            break;

        case KEY_DOWN:

            c = 0;
            if (win->buff_data->wrap_y >= (rows - 1)) {
                while (*(win->buff_data->buff_ptr + c) != '\n') {
                    ++c;
                }
                if (c <= cols) {
                    win->buff_data->buff_ptr += (c + 1);
                } else {
                    win->buff_data->buff_ptr += cols;
                }
            }
            break;

        case KEY_UP:

            c = 0;
            offset = 2;
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
                win->buff_data->buff_ptr = buff_ptr;
            }
            break;

        default:
            win->buff_data->buff_ptr = win->buff_data->buff;
    }

    wmove     (win->DWIN, 0, 0); 
    waddstr   (win->DWIN, win->buff_data->buff_ptr);
    getyx     (win->DWIN, win->buff_data->wrap_y, x);
    wclrtoeol (win->DWIN);
    wrefresh  (win->DWIN);
}
                


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



static int
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

    werase (win->DWIN);

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
        if ((fgets (line, sizeof (line), win->src_file_data->ptr)) == NULL) {
            pfem ("fgets error: \"%s\"", strerror (errno));
            pemr ("Unable to get source file line");
        }
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
    return A_OK;
}
*/


