#include <stdlib.h>
#include <ncurses.h>

#include "_plugins.h"


/*
    Navigate back

        e.g. unfocus window -> quit termIDE

*/
int termide_back (void)
{
    // close Ncurses properly
    endwin ();
    printf ("\nGoodbye!\n");
    exit (EXIT_SUCCESS);
}

