
/*
   gdb-tuiffic
 */

#include <string.h>
#include <locale.h>
#include <ncurses.h>

#include "data.h"
#include "parse_config.h"
#include "render_screen.h"
#include "run_plugin.h"
#include "plugins/_plugins.h"
#include "utilities.h"


static void create_colors ();
static layouts_t* allocate_layouts_struct (void);



int main (void) 
{
    int li;
    int ch;

    // initialize Ncurses
    initscr ();
    create_colors ();   // create color pairs
    cbreak ();          // disable need to press Enter after key choice
    noecho ();          // do not display pressed character

    // parse CONFIG_FILE data into layouts_t struct
    layouts_t *layouts = allocate_layouts_struct ();
    parse_config (layouts);

    // render first layout
    li = 0;
    render_screen (li, layouts);

    //
    //  Main loop
    //
        // read key
    while ((ch = getch()) != ERR) {

        // run plugin associated with key in CONFIG_FILE
        run_plugin (ch, li, layouts);
    }

    endwin ();
    return 0;
}



/*
   Create Ncurses FONT_BACKGROUND color pairs
*/
static void create_colors ()
{
    if (has_colors()) {

        start_color();

        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);
        init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);
        init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
        init_pair(WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);
    }
}



/*
    Allocate memory for layouts_t struct
*/
static layouts_t* allocate_layouts_struct (void)
{
    void *config_ptr = (layouts_t*) malloc (sizeof (layouts_t));
    ((layouts_t *) config_ptr)->num = 0;

    if (config_ptr == NULL) {
        endwin ();
        pfem ("layouts_t allocation failed\n");
        exit (EXIT_FAILURE);
    } else {
        return config_ptr;
    }
}

