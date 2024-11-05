#ifndef GET_ASSEMBLY_DATA_H
#define GET_ASSEMBLY_DATA_H

#include "../data.h"



/*
    Get assembly data
    -------
    state->plugins[Asm]->win->buff_data
    debugger->curr_Asm_line
*/
int get_assembly_data (state_t *state);



#endif
