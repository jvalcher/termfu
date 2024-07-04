#include <signal.h>
#include <ncurses.h>
#include <termio.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

#include "data.h"
#include "utilities.h"
#include "parse_config_file.h"
#include "render_layout.h"
#include "start_debugger.h"
#include "run_plugin.h"

static void         parse_cli_arguments     (int, char *argv[], debugger_t*);
static void         initialize_ncurses      (void);
static void         set_signals_et_al       (void);
static void         handle_sigint_exit      (int);



int main (int argc, char *argv[]) 
{
    int key;

    state_t state;
    debugger_t debugger;
    state.debugger = &debugger;

    parse_cli_arguments (argc, argv, &debugger);

    initialize_ncurses ();

    state.layouts = parse_config_file (&state);

    render_layout (FIRST_LAYOUT, &state);

    start_debugger (&state); 

    set_signals_et_al ();

    while (debugger.running) {
        key = getkey ();
        run_plugin (state.plugin_key_index[key], &state);
    }

    clean_up ();

    return 0;
}



/*
    Parse command-line arguments
*/
static void
parse_cli_arguments (int argc,
                     char *argv[],
                     debugger_t *debugger)
{
    // TODO: add flag options
        // -h  - help
        // -f  - config file path
        // -d  - manually choose debugger
        // -l  - set initial layout
    if (argc > 1) {
        debugger->prog_path = argv [1];
    } else {
        pfeme ("Usage:  termide a.out\n");
    }
}


    
/*
    Initialize Ncurses
*/
void
initialize_ncurses (void)
{
#ifndef DEBUG

    initscr();         // initialize Ncurses
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
    cbreak();          // disable need to press Enter after key choice
    noecho();          // do not display pressed key character
    curs_set (0);      // hide cursor

#endif
}



/*
    Set signals
*/
static void
set_signals_et_al (void)
{
    // ctrl + c
    signal (SIGINT, handle_sigint_exit);
}



/*
    Intercept SIGINT (Ctrl-c) -> exit Ncurses, program
*/
void handle_sigint_exit (int sig_num)
{
    (void) sig_num;

    // TODO: send >EXIT et al. to processes
    pfeme ("Program exited (SIGINT)\n");
}



