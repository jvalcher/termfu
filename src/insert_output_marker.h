
#ifndef INSERT_OUTPUT_MARKER_H
#define INSERT_OUTPUT_MARKER_H

#include "data.h"


/*
   Insert debugger output markers before and after commands
   -------
   Ensures all debugger output is read when multiple read()s required
   in parse_debugger_output()
*/
void  insert_output_start_marker (state_t* state);
void  insert_output_end_marker   (state_t* state);



#endif
