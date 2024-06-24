
#ifndef _INTERFACE_H
#define _INTERFACE_H


/*
    Plugin function et al. interface
    ---------
    - Debugger et al. plugin functions
    - Other functions specific to each debugger
*/

#include "../data.h"



/****************
  Misc Functions
 ****************/


void insert_output_start_marker (char*, state_t*);
void insert_output_end_marker   (char*, state_t*);
void parse_output               (int*, int, char*, char*, char*, char*);
void run_other_win_key          (int, state_t*);



/******************
  Plugin functions
 ******************/


/*
   Misc
*/

void empty_func      (state_t*);

// window
void scroll_up       (state_t*);
void scroll_down     (state_t*);
void scroll_left     (state_t*);
void scroll_right    (state_t*);

// popup window
void pwin_builds     (state_t*);
void pwin_layouts    (state_t*);



/*
    Debugger
*/

// header
void back            (state_t*);
void quit            (state_t*);
void cont            (state_t*);
void finish          (state_t*);
void kill_prog       (state_t*);
void next            (state_t*);
void run             (state_t*);
void step            (state_t*);
void until           (state_t*);

// window
void win_assembly    (state_t*);
void win_breakpoints (state_t*);
void win_local_vars  (state_t*);
void win_prog_output (state_t*);
void win_prompt      (state_t*);
void win_registers   (state_t*);
void win_src_file    (state_t*);
void win_watches     (state_t*);


#endif
