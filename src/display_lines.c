
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "display_lines.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "format_window_data.h"

#define LINE_NUM_TEXT_SPACES  2

static int create_scroll_buffer_llist (int plugin_index, state_t *state);



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



static int
create_scroll_buffer_llist (int      plugin_index,
                            state_t *state)
{
    int   chars,
          max_chars;
    char *ptr;
    window_t           *win;
    debugger_t         *debugger;
    buff_data_t        *buff_data;
    scroll_buff_line_t *buff_line,
                       *curr_buff_line,
                       *tmp_buff_line;

    // free scroll_buff_line_t linked list
    win = state->plugins[plugin_index]->win;
    debugger = state->debugger;
    buff_data = win->buff_data;
    buff_line = buff_data->head_line;
    while (buff_line != NULL) {
        tmp_buff_line = buff_line->next;
        free (buff_line); 
        buff_line = tmp_buff_line;
    }

    buff_data->head_line         = NULL;
    buff_data->tail_line         = NULL;
    buff_data->curr_line         = NULL;
    curr_buff_line               = NULL;
    buff_data->scroll_row        = 1;
    buff_data->scroll_col_offset = 0;
    buff_data->rows              = 0;

    // update source file buffer if path changed
    if (plugin_index == Src && debugger->src_path_changed) {

        if (buff_data->buff != NULL) {
            free (buff_data->buff);
        }

        buff_data->buff = create_buff_from_file (debugger->src_path_buffer);
        if (buff_data->buff == NULL) {
            buff_data->buff = create_buff_from_file (debugger->main_src_path_buffer);
            if (buff_data->buff == NULL) {
                pfemr ("Failed to create buffer from file \"%s\"", debugger->main_src_path_buffer);
            }
        }

        debugger->src_path_changed = false;
    }

    // create linked list from buffer
    ptr       = buff_data->buff;
    max_chars = 0;
        //
    do {

        buff_data->rows += 1;

        // allocate scroll_buff_line_t
        if ((buff_line = (scroll_buff_line_t*) malloc (sizeof (scroll_buff_line_t))) == NULL) {
            pfem ("malloc error: \"%s\"", strerror (errno));
            pemr ("Failed to allocate scroll_buff_line_t");
        }

        // add to linked list
        if (buff_data->head_line == NULL) {
            buff_data->head_line = buff_line;
            curr_buff_line       = buff_line;
            curr_buff_line->prev = NULL;
            curr_buff_line->line = 1;
        } else {
            buff_line->line      = curr_buff_line->line + 1;
            buff_line->prev      = curr_buff_line;
            curr_buff_line->next = buff_line;
            curr_buff_line       = buff_line;
        }
        curr_buff_line->next = NULL;

        // line start
        curr_buff_line->ptr = ptr;

        // number of characters
        chars = 1;
        while (*ptr != '\n' && *ptr != '\0') {

            // split line
            if (buff_data->text_wrapped &&
                chars >= win->data_win_cols)
            {
                break;
            }

            ++chars;
            ++ptr;
        }
        curr_buff_line->len = chars;

        // max line length
        if (chars > max_chars) {
            max_chars = chars;
        }

        // skip newline character
        if (*ptr == '\n') {
            ++ptr;
        }
        
    } while (*ptr != '\0');

    buff_data->max_chars = max_chars;
    buff_data->tail_line = curr_buff_line;
    buff_data->rows      = curr_buff_line->line;

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
          last_line_num = -1,
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
            case Reg:

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

