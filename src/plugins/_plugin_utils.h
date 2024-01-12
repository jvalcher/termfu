
#ifndef plugins_h
#define plugins_h

#include <ncurses.h>
#include "../data.h"



/*
    Switch string's colors in Ncurses WINDOW element for quarter second
    ------------
    - Used at beginning of plugin functions with title string in header 
      to indicate usage
*/
void pulse_window_string (char *code, layout_t *layout);



/*
    Find string in Ncurses WINDOW 
    -----------
    - Set y,x to coordinates if found
    - Otherwise set both to -1
*/
void find_window_string (WINDOW *window, char *string, int *y, int *x);


/*
    Get title string for plugin code in current layout
    ----------
    Nxt -> (n)ext
*/
char *get_code_title (char *code, layout_t *layout);



#endif
