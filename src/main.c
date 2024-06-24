#include <signal.h>
#include <ncurses.h>
#include <termio.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>

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
static void      set_signal_semaphore   (state_t*);
static void      handle_sigint_exit     (int);
static char      getchr                 (void);



int main (int argc, char *argv[]) 
{
    state_t *state = allocate_state();

    parse_cli_arguments (argc, argv, state);

    initialize_ncurses();

    set_signal_semaphore (state);

    parse_config (state);

    render_layout (first_layout, state);

    bind_keys_wins_paths (state);

    start_debugger (state); 

    state->running = true;

    while (state->running) {
        int ch = getchr();
        run_plugin (ch, state);
    }

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



static debug_state_t *allocate_debug_state (void)
{
    debug_state_t *dstate = (debug_state_t*) malloc (sizeof (debug_state_t));
    if (dstate == NULL) {
        pfeme ("debug_state_t allocation error\n");
    }
    return dstate;
}



/*
    Parse command-line arguments
*/
static void parse_cli_arguments (int argc,
                                 char *argv[],
                                 state_t *state)
{
    state->debug_state = allocate_debug_state ();

    // TODO: add flag options
        // -h  - help
        // -f  - config file path
        // -d  - manually choose debugger
        // -l  - set initial layout
    if (argc > 1) {
        state->debug_state->prog_path = argv [1];
    } else {
        pfeme ("Usage:  termide a.out\n");
    }
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
static void set_signal_semaphore (state_t *state)
{
    int sem_val;

    // ctrl + c
    signal (SIGINT, handle_sigint_exit);

    // render_window_data() sempahore
    state->reader_sem = sem_open (RENDER_WINDOW_SEM_NAME, O_CREAT, 0600, 1);
    sem_getvalue (state->reader_sem, &sem_val);
    if (sem_val == 0) {
        sem_post (state->reader_sem);
    }
    state->process = PARENT_PROCESS;
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



static char getchr (void)
{
#ifndef DEBUG

        return getch();

#endif

#ifdef DEBUG

    int ch,
        stdout_fd,
        dev_null_fd;
    static struct termios oldt, newt;

    dev_null_fd = open ("/dev/null", O_WRONLY);
    stdout_fd   = dup (STDOUT_FILENO);

    // disable need to hit enter after key press
    tcgetattr (STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);          
    tcsetattr (STDIN_FILENO, TCSANOW, &newt);
    dup2 (dev_null_fd, STDOUT_FILENO);
        //
    ch = getchar();
        //
    dup2 (stdout_fd, STDOUT_FILENO);
    tcsetattr (STDIN_FILENO, TCSANOW, &oldt);

    close (dev_null_fd);
    return ch;

#endif
}
