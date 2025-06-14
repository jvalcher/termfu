#ifndef TEST_UTILITIES_H
#define TEST_UTILITIES_H

#include "../src/data.h"



/*
    Pause process, print PID for debugger to attach to
*/
void process_pause (void);



/*
    Start termfu, debugger
*/ 
state_t *start_termfu (int argc, char *argv[]);



/*
    Allocate state_t, debugger_t, state->plugins[index]
*/
state_t* allocate_test_plugin(int index);



/*
    Initialize ncurses
*/
void init_nc (void);
void deinit_nc (void);



/*
   Create ncurses window with data subwindow
   - state->plugins[I]->win->WIN, ->DWIN
   - Set test_* values to change defaults
*/
extern int test_cols;
extern int test_rows;
extern int test_y;
extern int test_x;
    //
void create_nc_window (window_t *win);



/*
    Print formatted string to DEBUG_OUT_TEST_PATH 
*/
#define DEBUG_OUT_TEST_PATH "tests/debug.out"
    //
void logt (const char *formatted_string, ...);



/*
    Print debugger state to DEBUG_OUT_TEST_PATH
*/
void print_debugger_state_test (state_t *state);



#endif
