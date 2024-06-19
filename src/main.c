#include <signal.h>
#include <ncurses.h>

#include "data.h"
#include "utilities.h"
#include "create_colors.h"
#include "parse_config.h"
#include "render_layout.h"
#include "bind_keys_wins_paths.h"
#include "start_debugger.h"
#include "run_plugin.h"

static state_t  *allocate_state         (void);
static void      parse_cli_arguments    (int, char *argv[], state_t*);
static void      initialize_ncurses     (void);
static void      set_signals            (void);
static void      handle_sigint_exit     (int);



int main (int argc, char *argv[]) 
{
    int   ch;
    state_t *state;

    state = allocate_state();

    parse_cli_arguments (argc, argv, state);

    initialize_ncurses();

    set_signals();

    parse_config (state);

    render_layout (first_layout, state);

    bind_keys_wins_paths (state);

    start_debugger (state); 

    while ((ch = getch()) != ERR && state->running)
        run_plugin (ch, state);

    close_ncurses();
    return 0;
}



/*
    Allocate memory for state_t struct
*/
static state_t *allocate_state (void)
{
    state_t *state = (state_t*) malloc (sizeof (state_t));
    if (state == NULL) {
        pfeme ("data_t allocation error\n");
    }
    return state;
}



/*
    Parse command-line arguments
*/
static void parse_cli_arguments (int argc,
                             char *argv[],
                             state_t *state)
{
    // TODO: add flag options
        // -h  - help
        // -f  - config file path
        // -d  - manually choose debugger
        // -l  - set initial layout
    if (argc > 1) 
        state->debug_state->prog_path = argv [1];
    else 
        pfeme ("Usage:  termide a.out\n");
}


    
/*
    Initialize Ncurses
*/
void initialize_ncurses (void)
{
#ifndef DEBUG
    initscr();         // initialize Ncurses
    create_colors();   // create color pairs
    cbreak();          // disable need to press Enter after key choice
    noecho();          // do not display pressed key character
    curs_set (0);      // hide cursor
#endif
}



/*
    Set signals
*/
static void set_signals (void)
{
    signal (SIGINT, handle_sigint_exit);
}



/*
    Intercept SIGINT (Ctrl-c) -> exit Ncurses, program
*/
void handle_sigint_exit (int sig_num)
{
    pfeme ("Program exited (SIGINT)\n");
}



