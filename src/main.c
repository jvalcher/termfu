#include <unistd.h>
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

static void *get_key           (void *state_arg);
static void *send_key          (void *state_arg);

int  key_pipe[2];
bool in_select_window;
pthread_mutex_t mutex;
pthread_cond_t cond_var;



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



/*

TODO: Decouple, reorganize code
TODO: Replace switch tables with dynamic dispatching and lookup table
TODO: Move descriptions next to variables in data.h
TODO: Create plugin type boolean functions (is window, form?), replace switch sections
TODO: Add example gdb/mi output for all functions
TODO: Add command history (persisted, limit)
BUG: Figure out startup bug where doesn't always load debugger
FIXME: Wait for debugger process to start before updating window data
TODO: Add function benchmarks, history
PERF: store dereferences in variables (?)
TODO: Add termfu tips back in (config copy, vim breakpoints, ...)
TODO: Add ">>> <cmd>" to Dbg output
FIXME: Fix make plugins
FIXME: Fix C++ class watches, add C++ test programs
FIXME: Fix valgrind leaks
TODO: Switch to semantic versioning, add release link to CONTRIBUTING.md, add RELEASE.md, patch versioning (v1.2.3 -> v1.2)

*/
