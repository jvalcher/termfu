#ifndef GET_SOURCE_FILE_PATH_H
#define GET_SOURCE_FILE_PATH_H

#include "../data.h"



/*
    Source file absolute path --> state->plugins[Src]->win->file_data->path
    Current line number       --> state->plugins[Src]->win->file_data->line
    If path changed...        --> state->plugins[Src]->win->file_data->path_changed
*/
int get_source_path_line_memory (state_t *state);



#endif
