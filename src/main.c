
//
// TODO: re-render layout on terminal screen size change
// TODO: put parse_configuration_file() through its paces; add more error handling
//

#include <unistd.h>
#include <signal.h>
#include <ncurses.h>
#include <errno.h>

#include "data.h"
#include "utilities.h"
#include "parse_config_file.h"
#include "render_layout.h"
#include "start_debugger.h"
#include "run_plugin.h"
#include "persist_data.h"
#include "update_window_data/_update_window_data.h"
#include "plugins.h"

static int  initial_configure   (int, char*[], state_t*);
static void exit_signal_handler (int sig_num);



int
main (int   argc,
      char *argv[]) 
{
    int        key, ret;
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

    ret = render_layout (FIRST_LAYOUT, &state);
    if (ret == FAIL) {
        pfeme ("Failed to render \"%s\" layout\n\n", FIRST_LAYOUT);
    }

    ret = start_debugger (&state);
    if (ret == FAIL) {
        pfeme ("Failed to start debugger");
    }

    ret = update_windows (&state, 2, Src, Asm);
    if (ret == FAIL) {
        pfeme ("Failed to update windows");
    }

    ret = get_persisted_data (&state);
    if (ret == FAIL) {
        pfeme ("Failed to get persisted data");
    }

    while (debugger.running) {

        key = getch ();

        if (key == ESC) {
            break;
        }

        ret = run_plugin (state.plugin_key_index[key], &state);
        if (ret == FAIL) {
            pfeme ("Failed to run plugin for key \"%c\" (%d)", key, key);
        }
    }

    ret = persist_data (&state);
    if (ret == FAIL) {
        pfeme ("Failed to persist data");
    }

    clean_up ();

    return EXIT_SUCCESS;
}



/*
    Initial configuration
    ---------

    - CLI flags

        -h
            - Prints usage instructions

        -c  <Configuration file path>
            - See README.md for more information
            - default: CONFIG_FILE
            - Outputs to state->config_path

        -p  <Data persistence file path>
            - Persists watchpoints, breakpoints
            - default: PERSIST_FILE
            - Outputs to state->data_path

        -d
            - Wait for debugger to attach

    - Set state pointer
    - Set signals
    - Initialize Ncurses
*/
static int
initial_configure (int   argc,
                   char *argv[],
                   state_t *state)
{
    int opt;
    char pid_buf [8];
    extern char *optarg;
    bool debugging_mode;
    FILE *fp;

    //
    // CLI arguments
    //
    state->config_path[0] = '\0';
    state->data_path[0]   = '\0';
    debugging_mode = false;
    char *optstring = "hdc:p:";

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
                "\n"
                "       -d                Start in debug mode\n"
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

            // debugger mode
            case 'd':
                debugging_mode = true;
                break;

            default:
                fprintf (stderr,

                "\n"
                "Run with -h flag to see usage instructions.\n"
                "\n");

                exit (EXIT_FAILURE);
        }
    }


    // debugging mode
    if (debugging_mode) {

        // write debugged termfu PID to DEBUG_PID_FILE
        fp = fopen (DEBUG_PID_FILE, "w");
        if (fp == NULL) {
            pfem ("fopen error: %s", strerror (errno));
            pemr ("Unable to open debug PID file \"%s\"\n", DEBUG_PID_FILE);
        }
        fprintf (fp, "%ld", (long) getpid());
        fclose (fp);

        // copy PID to clipboard
        sprintf (pid_buf, "%ld", (long) getpid());
        copy_to_clipboard (pid_buf);

        // print message
        printf (
            "\n"
            "\033[0;32mDebugged process ID: \033[0;36m%s\033[0m\n"
            "\n"
            "   - Copied to \033[0;36m%s\033[0m\n"
            "   - Copied to clipboard\n"
            "\n"
            "\033[0;32mAttach to this process with debugger\033[0m\n"
            "\n"
            "   - With new debugger session automatically through \033[0;36m%s\033[0m\n"
            "\n"
            "       $ make conn_proc_<debugger>\n"
            "\n"
            "   - With existing debugger session, paste PID currently in\n"
            "     clipboard into prompt\033[0m\n"
            "\n"
            "       >> (at)tach <pid>\n"
            "\n"
            "\033[0;32mContinue execution with debugger\033[0m\n"
            "\n"
            "   - Set breakpoint after this message string in \033[0;36mmain.c\033[0m, i.e. after \033[0;36mgetchar()\033[0m\n"
            "   - Continue execution\n"
            "\n"
            "\033[0;33mPress any key to continue...\033[0m\n"
            "\n"
            "\033[0;32mSend exit signal to debugged process\033[0m\n"
            "\n"
            "   >> signal 2\n"
            "\n"
            "\033[0;32mOr refresh terminal pane\033[0m\n"
            "\n",
            pid_buf, DEBUG_PID_FILE, DEBUG_PID_FILE);

        // wait...
        getchar ();
    }


    state->new_run = true;
    

    // Set state pointer in utilities.c for exiting ncurses, persisting data with clean_up()
    set_state_ptr (state);


    // signal handlers
    signal (SIGINT, exit_signal_handler);     // Ctrl-C;  (gdb) signal 2


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
    } else {
        pfemr ("Terminal doesn't support colors\n");
    }
    cbreak ();
    noecho ();
    curs_set (0);
    set_escdelay (0);

    return A_OK;
}



static void
exit_signal_handler (int sig_num)
{
    (void) sig_num;
    clean_up ();
    fprintf (stderr, "termfu exited (SIGINT)\n");
    exit (EXIT_FAILURE);
}

