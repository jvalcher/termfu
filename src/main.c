#include <signal.h>
#include <ncurses.h>

#include "data.h"
#include "utilities.h"
#include "create_colors.h"
#include "parse_config.h"
#include "render_layout.h"
#include "bind_keys_windows.h"
#include "start_debugger.h"
#include "run_plugin.h"

static void           sigint_exit          (int sig_num);
static void           initialize_ncurses   (void);
static debug_state_t *allocate_debug_state (void);
static state_t       *allocate_state       (void);



/*
    Main
*/
int main (int argc, char *argv[]) 
{
    int   ch;
    state_t *state     = allocate_state ();
    state->debug_state = allocate_debug_state ();

    if (argc > 1) 
        state->debug_state->prog_path = argv [1];
    else 
        pfeme ("Usage:  termide a.out\n");
    
    // TODO: manual data
    char *static_break = "main";
    state->debug_state->break_point = static_break;
    state->debug_state->debugger = DEBUGGER_GDB;

    initialize_ncurses();
    signal (SIGINT, sigint_exit);
    parse_config (state);
    render_layout ("first", state);
    bind_keys_windows_to_plugins (state);
    start_debugger (state->debug_state); 

    while ((ch = getch()) != ERR && state->debug_state->running)
        run_plugin (ch, state);

    curs_set (1);
    endwin ();
    return 0;
}



/*
    Intercept SIGINT (Ctrl-c) -> exit Ncurses, program
*/
void sigint_exit (int sig_num)
{
    pfeme ("Program exited (SIGINT)\n");
}



/*
    Initialize Ncurses
*/
void initialize_ncurses (void)
{
    initscr ();         // initialize Ncurses
    create_colors ();   // create color pairs
    cbreak ();          // disable need to press Enter after key choice
    noecho ();          // do not display pressed character
    curs_set(0);        // hide cursor
}



/*
    Allocate memory for debug_state_t struct
*/
static debug_state_t *allocate_debug_state (void)
{
    debug_state_t *dstate = (debug_state_t*) malloc (sizeof (debug_state_t));
    if (dstate == NULL) {
        pfeme ("debug_state_t allocation error\n");
    }
    return dstate;
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

