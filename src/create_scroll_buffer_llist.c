#include <stdlib.h>

#include "data.h"
#include "utilities.h"
#include "plugins.h"



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

    buff_data->head_line         = NULL;
    buff_data->tail_line         = NULL;
    buff_data->curr_line         = NULL;
    curr_buff_line               = NULL;
    buff_data->scroll_row        = 1;
    buff_data->scroll_col_offset = 0;
    buff_data->rows              = 0;

    // update source file buffer if path changed
    if (plugin_index == Src && state->debugger->src_path_changed) {

        if (buff_data->buff != NULL) {
            free (buff_data->buff);
        }

        buff_data->buff = create_buff_from_file (state->debugger->src_path_buffer);
        if (buff_data->buff == NULL) {
            pfemr ("Failed to create buffer from file \"%s\"", state->debugger->src_path_buffer);
        }

        state->debugger->src_path_changed = false;
    }

    ptr       = buff_data->buff;
    max_chars = 0;

    // create linked list from buffer
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

