#ifndef CREATE_SCROLL_BUFFER_LLIST_H
#define CREATE_SCROLL_BUFFER_LLIST_H

#include "data.h"



/*
    Create scroll_buff_line_t linked list
    -------
    - Buffer:
        state->plugins[i]->win->buff_data->buff
    - Linked list:
        state->plugins[i]->win->buff_data->head_line, curr_line, tail_line
*/
int create_scroll_buffer_llist (int plugin_index, state_t *state);



#endif
