
/*
   gdb-tuiffic
 */

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <ncurses.h>

#include "data.h"
#include "parse_config.h"
#include "render_screen.h"
#include "utilities.h"


static void create_colors ();
static layouts_t* allocate_layouts_struct (void);



int main (void) 
{
    // initialize Ncurses, create color pairs
    initscr ();
    create_colors ();

    // create data structures to hold parsed CONFIG_FILE data (parse_config.h)
    layouts_t *layouts = allocate_layouts_struct ();

    // parse configuration file
    parse_config (layouts);

    // render screen
    int li = 0;                 // index of first layout
    render_screen (li, layouts);

    //
    //  Main loop
    //  ---------
    //  q   - quit
    //
    int ch;
    cbreak ();      // disable need to press Enter after key choice
    noecho ();      // do not display pressed character

        // read key presses
    while ((ch = getch()) != ERR) {

        switch (ch) {

            // quit
            case 'q':
                goto exit;

            // render screen
            default:
                render_screen (li, layouts);
                break;
        }
    }

exit:
    endwin ();
    return 0;
}



/*
   Create font_background color pairs
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
    Allocate memory for configuration structs
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

