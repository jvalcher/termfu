#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <sys/wait.h>
#include <pthread.h>

#include "data.h"
#include "utilities.h"
#include "parse_config_file.h"
#include "render_layout.h"
#include "start_debugger.h"
#include "run_plugin.h"
#include "persist_data.h"
#include "plugins.h"

static int   initial_configure (int, char*[], state_t*);
static int   start_program     (state_t *state);
static void *get_key           (void *state_arg);
static void *send_key          (void *state_arg);

int main_pipe[2];
pthread_mutex_t mutex;
pthread_cond_t cond_var;
bool in_select_window;

// FIX: termfu_dev (not termfu) throwing a "free(): invalid size" error in fedora 41 VM



int
main (int   argc,
      char *argv[]) 
{
    int        ret,
               status;
    state_t    state;
    debugger_t debugger;

    state.debugger = &debugger;

    ret = initial_configure (argc, argv, &state);
    if (ret == FAIL) {
        pfeme ("Initial configuration failed\n\n");
    }

    ret = parse_config_file (&state);
    if (ret== FAIL) {
        pfeme ("Failed to parse configuration file\n\n");
    }

    ret = start_program (&state);
    if (ret == FAIL) {
        pfeme ("Failed to start termfu");
    }

    while (debugger.running) {

        // create main, select pipes
        if (pipe (main_pipe) == -1) {
            pfeme ("Failed to create main pipe");
        }

        // start thread to get key input
        if (pthread_create(&state.debugger->get_key_thread, NULL, get_key, (void*) &state) != 0) {
            pfeme ("Failed to create get key thread");
        }

        // start thread to send key to plugin
        if (pthread_create(&state.debugger->send_key_thread, NULL, send_key, (void*) &state) != 0) {
            pfeme ("Failed to create run plugin thread");
        }

        pthread_join (state.debugger->get_key_thread, NULL);
        pthread_join (state.debugger->send_key_thread, NULL);

        // restart program
        if (state.restart_prog) {

            // kill debugger process
            kill    (debugger.pid, SIGTERM);
            waitpid (debugger.pid, &status, 0);

            // restart program
            ret = start_program (&state);
            if (ret == FAIL) {
                pfeme ("Failed to start termfu");
            }

            state.restart_prog = false;
        } 

        // quit program
        else {
            debugger.running = false;
        }

        close (main_pipe[PIPE_READ]);
        close (main_pipe[PIPE_WRITE]);
    }

    clean_up ();

    return EXIT_SUCCESS;
}



/*
    SIGINT handler (Ctrl-C)
*/
static void
sigint_handler (int sig_num)
{
    (void) sig_num;
    clean_up ();
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

    // CLI arguments
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
    } else {
        pfemr ("Terminal doesn't support colors\n");
    }
    cbreak ();
    noecho ();
    curs_set (0);
    set_escdelay (0);
    keypad (stdscr, TRUE);

    return A_OK;
}



static int
start_program (state_t *state)
{
    int ret;

    ret = render_layout (FIRST_LAYOUT, state);
    if (ret == FAIL) {
        pfeme ("Failed to render \"%s\" layout\n\n", FIRST_LAYOUT);
    }

    ret = start_debugger (state);
    if (ret == FAIL) {
        pfeme ("Failed to start debugger");
    }

    ret = get_persisted_data (state);
    if (ret == FAIL) {
        pfeme ("Failed to get persisted data");
    }

    return A_OK;
}

/*
    Get key input (thread function)
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
        while (in_select_window == true) {
            pthread_cond_wait (&cond_var, &mutex);
        }
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
            pthread_cancel (state->debugger->send_key_thread);
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
            if (write (main_pipe[PIPE_WRITE], key_str, 8) == -1) {
                pfem  ("write failure: \"%s\"", strerror (errno));
                pfeme ("Failed to write to main pipe");
            };
        }

    }

    return NULL;
}



/*
    Send key input to run_plugin() (thread function)
*/
static void*
send_key (void *state_arg)
{
    char     key_str[8];
    int      key,
             ret,
             oldtype;
    state_t *state;

    pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
    state = (state_t*) state_arg;

    while (true) {

        // read key from get_key()
        if (read (main_pipe[PIPE_READ], key_str, 8) > 0) {

            key = atoi (key_str);

            // exit on Esc
            if (key == ESC) {
                state->debugger->running = false;
            }

            // run plugin
            else if ((key >= 'A' && key <= 'Z') || (key >= 'a' && key <= 'z')) {

                ret = run_plugin (state->plugin_key_index[key], state);
                if (ret == FAIL) {
                    state->debugger->running = false;
                }

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

            // quit program
            if (state->debugger->running == false) {
                pthread_cancel (state->debugger->get_key_thread);
                pthread_exit (&ret);
            }
        }
    }

    return NULL;
}

