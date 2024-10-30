
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "display_lines.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "format_window_data.h"
#include "create_scroll_buffer_llist.h"

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

    // erase window
    werase (win->DWIN);

    // set curr_line
    if (buff_data->curr_line == NULL) {
        buff_data->curr_line = buff_data->head_line;
    }

    // calculate line number offset for Src
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
                 buff_data->curr_line  = buff_data->head_line;
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
            if (buff_data->text_wrapped == false) {
                if (buff_data->scroll_col_offset > 0) {
                    --buff_data->scroll_col_offset;
                }
            }
            break;

        case 'l':
        case KEY_RIGHT:
            if (buff_data->text_wrapped == false) {
                if ((buff_data->max_chars - buff_data->scroll_col_offset) >
                    (win->data_win_cols - line_num_len))
                {
                    ++buff_data->scroll_col_offset;
                }
            }
            break;
    }
    
    // print lines
    i = 0;
    buff_line = buff_data->curr_line;
    text_len = win->data_win_cols - line_num_len;

    while (i < win->data_win_rows && buff_line != NULL) {

        switch (win->index) {

            case Asm:

                print_len = buff_line->len - buff_data->scroll_col_offset;
                if (print_len <= 0) {
                    ptr = blank_line; 
                    print_len = 1;
                } else {
                    ptr = buff_line->ptr + buff_data->scroll_col_offset;
                }
                mvwprintw (win->DWIN, i, 0, "%.*s", print_len, ptr);

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
                mvwprintw  (win->DWIN, i, 0, "%d%*c", buff_line->line, rem_spaces, ' ');
                wattroff (win->DWIN, COLOR_PAIR(SRC_LINE_COLOR));

                // text
                print_len = buff_line->len - buff_data->scroll_col_offset;
                if (print_len <= 0) {
                    ptr = blank_line; 
                    print_len = 1;
                } else {
                    ptr = buff_line->ptr + buff_data->scroll_col_offset;
                    if (print_len >= text_len) {
                        print_len = text_len;
                    }
                }
                mvwprintw (win->DWIN, i, line_num_len, "%.*s", print_len, ptr);

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

