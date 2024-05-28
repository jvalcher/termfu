
/*
   gdb-tuiffic
 */

#include <stdlib.h>
#include <signal.h>
#include <ncurses.h>

#include "data.h"
#include "parse_config.h"
#include "render_layout.h"
#include "run_debugger.h"
#include "run_plugin.h"
#include "utilities.h"
#include "bind_keys_windows.h"


static void create_colors ();
void sigint_exit (int sig_num);
char *binary_name;      // binary name argument passed to termIDE


int main (int argc, char *argv[]) 
{
    layout_t *curr_layout;
    int       ch;

    // set program name
    if (argc > 1) {
        binary_name = argv [1];
        printf ("%s\n", binary_name);
    } else {
        pfeme ("Usage:  termide a.out\n");
    }

    // initialize, configure Ncurses
    initscr ();         // initialize Ncurses
    create_colors ();   // create color pairs
    cbreak ();          // disable need to press Enter after key choice
    noecho ();          // do not display pressed character
    curs_set(0);        // hide cursor

    // exit gracefully on SIGINT
    signal (SIGINT, sigint_exit);

    // parse CONFIG_FILE data into layouts_t structs  (data.h)
    curr_layout = parse_config ();

    // render first layout in config file
    render_layout (curr_layout);

    // Bind keys, windows to plugin_t structs
    bind_keys_windows_to_plugins (curr_layout);

    // start debugger, load binary
    run_debugger (curr_layout, binary_name); 

    //  Main loop
    //
        // read key
    while ((ch = getch()) != ERR) {

        // run plugin
        run_plugin (ch, curr_layout);
    }

    curs_set (1);
    endwin ();
    return 0;
}



/*
   Create Ncurses <FONT_BACKGROUND> color pairs
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
    Intercept SIGINT (Ctrl-c) -> exit Ncurses, program
*/
void sigint_exit (int sig_num)
{
    pfeme ("Program exited (SIGINT)\n");
}
