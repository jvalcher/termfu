
/*
   gdb-tuiffic
 */

#include <string.h>
#include <locale.h>
#include <signal.h>
#include <ncurses.h>

#include "data.h"
#include "parse_config.h"
#include "render_layout.h"
#include "run_plugin.h"
#include "utilities.h"


static void create_colors ();
void sigint_exit (int sig_num);

char *binary_name;      // binary name argument passed to termIDE
int num_layouts;


int main (int argc, char *argv[]) 
{
    int ch;
    layout_t* first_layout;
    layout_t* curr_layout;

    // set program name
    if (argc > 1)
        binary_name = argv [1];

    // exit gracefully on SIGINT
    signal (SIGINT, sigint_exit);

    // initialize, configure Ncurses
    initscr ();         // initialize Ncurses
    create_colors ();   // create color pairs
    cbreak ();          // disable need to press Enter after key choice
    noecho ();          // do not display pressed character
    curs_set(0);        // hide cursor

    // parse CONFIG_FILE data into layouts_t structs  (data.h)
    num_layouts  = 0;
    first_layout = parse_config ();
    curr_layout  = first_layout;

    // render first layout
    render_layout (curr_layout);

    //
    //  Main loop
    //
        // read key
    while ((ch = getch()) != ERR) {

        // run plugin
        run_plugin (ch, curr_layout);
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
        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);           // RED_BLACK
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);       // GREEN_BLACK
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);     // YELLOW_BLACK
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);         // BLUE_BLACK
        init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);   // MAGENTA_BLACK
        init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);         // CYAN_BLACK
        init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);       // WHITE_BLACK
        init_pair(WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);         // WHITE_BLUE
    }
}



/*
    Intercept SIGINT (Ctrl-c) and exit Ncurses gracefully
*/
void sigint_exit (int sig_num)
{
    pfeme ("Program exited (SIGINT)\n");
}
