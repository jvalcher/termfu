#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <sys/wait.h>
#include <pthread.h>

#include "data.h"
#include "utilities.h"
#include "error.h"
#include "parse_config_file.h"
#include "update_window_data/_update_window_data.h"
#include "render_layout.h"
#include "start_debugger.h"
#include "run_plugin.h"
#include "persist_data.h"
#include "plugins.h"

static int   initial_configure (int, char*[], state_t*);
static void *get_key           (void *state_arg);
static void *send_key          (void *state_arg);

int  key_pipe[2];
bool in_select_window;
pthread_mutex_t mutex;
pthread_cond_t  cond_var;

// TODO: Organize tests; implement test_all
// TODO: Add function benchmarks
// FIX: Fix valgrind leaks
// TODO: Add command history (persisted, limit)
// FIX: Wait for debugger process to start before updating window data
// OPTIMIZE: store dereferences in variables (?)
// TODO: Add termfu tips back in (config copy, vim breakpoints, ...)
// TODO: Add ">>> <cmd>" to Dbg output



int
main (int   argc,
      char *argv[]) 
{
    int        dstatus;
    state_t    state;
    debugger_t debugger;

    state.debugger = &debugger;

    if (initial_configure (argc, argv, &state) == FAIL)
        pfeme ("Initial configuration failed");

    if (parse_config_file (&state) == FAIL)
        pfeme ("Failed to parse configuration file");

    if (render_layout (FIRST_LAYOUT, &state) == FAIL)
        pfeme ("Failed to render layout");

    if (pthread_create (&state.update_window_thread, NULL, &update_window_thread, (void*) &state) != 0)
        pfeme ("Failed to start update window thread");

    if (start_debugger (&state) == FAIL)
        pfeme ("Failed to start debugger process");

    if (get_persisted_data (&state) == FAIL)
        pfeme ("Failed to get persisted data");

    while (debugger.running) {

        if (pipe (key_pipe) == -1)
            pfeme ("Failed to create main pipe");

        if (pthread_create (&state.get_key_thread, NULL, &get_key, (void*) &state) != 0)
            pfeme ("Failed to create get key thread");

        if (pthread_create (&state.send_key_thread, NULL, &send_key, (void*) &state) != 0)
            pfeme ("Failed to create run plugin thread");

        pthread_join (state.get_key_thread, NULL);
        pthread_join (state.send_key_thread, NULL);

        if (state.restart_prog) {
            state.restart_prog = false;

            kill    (debugger.pid, SIGTERM);
            waitpid (debugger.pid, &dstatus, 0);
            if (WIFEXITED(dstatus) == false)
                pfeme ("Failed to kill debugger process");

            if (render_layout (FIRST_LAYOUT, &state) == FAIL)
                pfeme ("Failed to render layout");

            if (start_debugger (&state) == FAIL)
                pfeme ("Failed to start debugger process");

            if (get_persisted_data (&state) == FAIL)
                pfeme ("Failed to get persisted data");
        } 
        else
            debugger.running = false;

        close (key_pipe[PIPE_READ]);
        close (key_pipe[PIPE_WRITE]);
    }

    clean_up (PROG_EXIT);

    return EXIT_SUCCESS;
}



/*
    SIGINT handler for Ctrl-C
*/
static void
sigint_handler (int sig_num)
{
    (void) sig_num;
    clean_up (PROG_EXIT);
    fprintf (stderr, "termfu exited (SIGINT)\n");
    exit (EXIT_FAILURE);
}



/*
    Initial configuration
    ---------
    - CLI flags
    - Set state pointer
    - Set signals
    - Initialize ncurses
*/
static int
initial_configure (int   argc,
                   char *argv[],
                   state_t *state)
{
    int opt;
    extern char *optarg;

    state->config_path[0] = '\0';
    state->data_path[0]   = '\0';

    char *optstring = "hc:p:";

    while ((opt = getopt (argc, argv, optstring)) != -1) {
        switch (opt) {

            // help
            case 'h':
                printf (
                "\n"
                "Usage: \n"
                "\n"
                "   $ termfu\n"
                "\n"
                "       Run in same directory as a %s configuration file\n"
                "       Data persisted to ./%s\n"
                "\n"
                "   $ termfu [OPTION...]\n"
                "\n"
                "       -c CONFIG_FILE    Use this configuration file\n"
                "       -p PERSIST_FILE   Persist sessions with this file\n"
                "\n",
                CONFIG_FILE, PERSIST_FILE);
                exit (EXIT_SUCCESS);

            // configuration file
            case 'c':
                strncpy (state->config_path, optarg, CONFIG_PATH_LEN - 1);
                break;

            // data persist file
            case 'p':
                strncpy (state->data_path, optarg, DATA_PATH_LEN - 1);
                break;

            default:
                fprintf (stderr,
                "\n"
                "Run with -h flag to see usage instructions.\n"
                "\n");
                exit (EXIT_FAILURE);
        }
    }

    // misc state
    state->new_run = true;
    state->restart_prog = false;
    set_state_ptr (state);
    set_num_plugins (state);

    // signal handler(s)
    signal (SIGINT, sigint_handler);     // Ctrl-C;  (gdb) signal 2

    // ncurses
    initscr ();
    if (has_colors ()) {
        start_color();
        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);           // RED_BLACK
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);       // GREEN_BLACK
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);     // YELLOW_BLACK
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);         // BLUE_BLACK
        init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);   // MAGENTA_BLACK
        init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);         // CYAN_BLACK
        init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);       // WHITE_BLACK
        init_pair(WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);         // WHITE_BLUE
        init_pair(BLACK_BLUE, COLOR_BLACK, COLOR_BLUE);         // WHITE_BLUE
    } 

    cbreak ();
    noecho ();
    curs_set (0);
    set_escdelay (0);
    keypad (stdscr, TRUE);

    return A_OK;
}



/*
    Get key input thread function
*/
static void*
get_key (void *state_arg)
{
    int      key,
             ret_val,
             oldtype;
    char     key_str[8];
    state_t *state;

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
    in_select_window = false;
    state = (state_t*) state_arg;
    state->debugger->running_plugin = false;

    while (true) {

        // wait if in select_window()
        while (in_select_window == true)
            pthread_cond_wait (&cond_var, &mutex);
        pthread_mutex_unlock (&mutex);

        // get key
        key = getch ();

        // if program hung, restart debugger with kill, quit, escape
        if ( state->debugger->running_plugin &&
            (state->plugin_key_index[key] == Kil ||
             state->plugin_key_index[key] == Qut ||
                                      key == ESC))
        {
            state->restart_prog = true;
            pthread_cancel (state->send_key_thread);
            pthread_exit (&ret_val);
        }

        // send key if plugin not running
        else if (state->debugger->running_plugin == false) {

            // signal to wait for return from select_window() plugin
            switch (state->plugin_key_index[key]) {
                case Asm:
                case AtP:
                case Brk:
                case Dbg:
                case Lay:
                case LcV:
                case Prg:
                case Prm:
                case Reg:
                case Src:
                case Stk:
                case Unt:
                case Wat: 
                    in_select_window = true;
                    pthread_mutex_lock (&mutex);
            }

            sprintf (key_str, "%d", key);
            if (write (key_pipe[PIPE_WRITE], key_str, 8) == -1)
                pfeme_errno ("Failed to write to main key pipe");
        }

    }

    return NULL;
}



/*
    Send key input to run_plugin() thread function
*/
static void*
send_key (void *state_arg)
{
    char     key_str[8];
    int      key,
             ret_val,
             oldtype;
    state_t *state;

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
    state = (state_t*) state_arg;

    while (true) {

        // read key from get_key()
        if (read (key_pipe[PIPE_READ], key_str, 8) > 0) {

            key = atoi (key_str);

            // exit on Esc
            if (key == ESC) {
                state->debugger->running = false;
            }

            // run plugin
            else if ((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {

                if (run_plugin (state->plugin_key_index[key], state) == FAIL)
                    state->debugger->running = false;

                // signal get_key() that it has exited select_window(), get_form_input()
                switch (state->plugin_key_index[key]) {
                    case Asm:
                    case AtP:
                    case Brk:
                    case Dbg:
                    case Lay:
                    case LcV:
                    case Prg:
                    case Prm:
                    case Reg:
                    case Src:
                    case Stk:
                    case Unt:
                    case Wat: 
                        in_select_window = false;
                        pthread_cond_signal (&cond_var);
                        pthread_mutex_unlock (&mutex);
                }
            }

            // exit program
            if (state->debugger->running == false) {
                pthread_cancel (state->get_key_thread);
                pthread_exit (&ret_val);
            }
        }
    }

    return NULL;
}

