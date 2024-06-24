
#ifndef INSERT_OUTPUT_MARKER_H
#define INSERT_OUTPUT_MARKER_H

#include "../data.h"


void insert_output_start_marker (char*, state_t*);
void insert_output_end_marker (state_t*);
void insert_output_win_select_marker (char*, state_t*);
void insert_output_win_deselect_marker (char*, state_t*);


#endif
