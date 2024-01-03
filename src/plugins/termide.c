
#include <stdlib.h>
#include <ncurses.h>

#include "_plugin_utils.h"

/*
    Placeholder function at plugin_function[0]  (run_plugin.c)
    -----------
    - Allows 0 to stand for "unassigned" in key_function_index[]  (data.h)
    - Not called
*/
int empty_func (void) {};


/*
    Navigate back

        - unfocus window
        - quit termIDE
*/
int termide_back (void)
{
    // close Ncurses
    endwin ();

    exit (EXIT_SUCCESS);
}


int termide_layouts (void) {};
int termide_builds (void) {};
