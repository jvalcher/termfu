#include <stdlib.h>
#include <ncurses.h>

#include "../data.h"


/*
    Placeholder function at plugin_function[0]  (run_plugin.c)
    -----------
    - Allows 0 to stand for "unassigned" in key_function_index[]  (data.h)
    - Not called
*/
int empty_func (debug_state_t *dstate) 
{
    return 0;
};


/*
    Navigate back
    -----------
        - TODO: back and exit conditionals
        - unfocus window
        - quit termIDE
*/
int termide_back (debug_state_t *dstate)
{
    // back

    // close Ncurses
    curs_set (1);
    endwin ();
    exit (EXIT_SUCCESS);
}



/*
    Open window listing available layouts
*/
int termide_layouts (debug_state_t *dstate) 
{
    return 0;
}



int termide_builds (debug_state_t *dstate) 
{
    return 0;
};
