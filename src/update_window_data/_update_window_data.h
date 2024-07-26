
#ifndef UPDATE_WINDOW_H
#define UPDATE_WINDOW_H

#include "../data.h"



void  update_windows (state_t*, int, ...);
void  update_window (int, state_t*);
void  get_buff_rows_cols (window_t*);
void  display_lines_buff (int, window_t*);
void  get_num_file_rows_cols (window_t*);
void  display_lines_file (int, window_t*);


#endif
