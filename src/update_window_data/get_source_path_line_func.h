#ifndef GET_SOURCE_FILE_PATH_H
#define GET_SOURCE_FILE_PATH_H

#include "../data.h"



/*
    Get source file data
    ---------
    debugger->curr_func
    debugger->src_path_buffer
    debugger->curr_Src_line
    debugger->src_path_changed
*/
int get_source_path_line_func (state_t *state);



#endif
