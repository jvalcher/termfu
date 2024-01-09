
#include <stdlib.h>
#include <ncurses.h>

#include "../utilities.h"
#include "_plugin_utils.h"

/*
    Placeholder function at plugin_function[0]  (run_plugin.c)
    -----------
    - Allows 0 to stand for "unassigned" in key_function_index[]  (data.h)
    - Not called
*/
int empty_func (int li, layouts_t *layouts) 
{
    return 1;
};


/*
    Navigate back

        - unfocus window
        - quit termIDE
*/
int termide_back (int li,
                  layouts_t *layouts)
{
    pulse_window_string ("Bak", li, layouts);

    // close Ncurses
    endwin ();
    exit (EXIT_SUCCESS);
}



/*
    Open window listing available layouts
*/
int termide_layouts (int li, 
                     layouts_t *layouts) 
{
    pulse_window_string ("Lay", li, layouts);

    return 1;
}



int termide_builds (int li,
                    layouts_t *layouts) 
{
    pulse_window_string ("Bld", li, layouts);
    return 1;
};
