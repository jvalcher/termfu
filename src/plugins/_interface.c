
#include "gdb.h"
#include "../data.h"
#include "../render_window_data.h"
#include "../run_plugin.h"


/*
    Placeholder function at plugin_function[0]  (run_plugin.c)
    -----------
    - Allows 0 to stand for "unassigned" in key_function_index[]  (data.h)
*/
void empty_func (state_t *state) {}



/********************
  Debugger functions
 ********************/


/*
    Insert start marker, plugin output file path into debugger process output
    ------------
    - Called by run_plugin()
    - Output used by debugger reader process
*/
void insert_output_start_marker (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_insert_output_start_marker (state);
            break;
    }
}



/*
    Insert end marker, plugin code into debugger process output
    ------------
    - Called by run_plugin()
    - Output used by debugger reader process
*/
void insert_output_end_marker (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_insert_output_end_marker (state);
            break;
    }
}



/*
    Parse debugger process output
    ------------
    - Called in debugger reader process
*/
void parse_output (int  *rstate, 
                   int   debugger, 
                   char *in_buffer, 
                   char *debug_out_path, 
                   char *program_out_path, 
                   char *code)
{
    switch (debugger) {
        case (DEBUGGER_GDB):
            gdb_parse_output (rstate, in_buffer, debug_out_path, program_out_path, code);
            break;
    }
}



/*****************************
  Misc header, window actions
 *****************************/


void quit (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_exit (state);
            break;
    }

    state->running = false;
}



void back (state_t *state)
{
    // TODO: popup window in selected window
        // push/pop  curr_window linked list
    if (state->curr_plugin->window)
        deselect_window (state->curr_plugin->title, state->curr_plugin->window);
}



void scroll_down (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, KEY_DOWN);
    }
}



void scroll_up (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, KEY_UP);
    }
}



void scroll_left (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, KEY_LEFT);
    }
}



void scroll_right (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, KEY_RIGHT);
    }
}



void run_other_win_key (int key, state_t *state)
{
    window_t *win = state->curr_window;
    win_keys_t *k = state->win_keys;

    // Use k->back in window while loop as exit condition

    if (key == k->scroll_up) {
        render_window_data (win, KEY_UP);
    } 
    else if (key == k->scroll_down) {
        render_window_data (win, KEY_DOWN);
    } 
    else if (key == k->scroll_left) {
        render_window_data (win, KEY_LEFT);
    } 
    else if (key == k->scroll_right) {
        render_window_data (win, KEY_RIGHT);
    } 
    else if (key == k->quit) {
        quit (state); 
    } 
    else {
        // TODO: pg up/down, home, end
        switch (key) {
            case KEY_UP:
                render_window_data (win, KEY_UP);
                break;
            case KEY_DOWN:
                render_window_data (win, KEY_DOWN);
                break;
            case KEY_RIGHT:
                render_window_data (win, KEY_RIGHT);
                break;
            case KEY_LEFT:
                render_window_data (win, KEY_LEFT);
                break;
        }
    }
}




/******************
   Header debugger
 ******************/



void cont (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_continue (state);
            break;
    }
}



void finish (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_finish (state);
            break;
    }
}



void kill_prog (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_kill (state);
            break;
    }
}



void next (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_next (state);
            break;
    }
}



void run (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_run (state);
            break;
    }
}



void step (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_step (state);
            break;
    }
}



// TODO: 
    // add line int to command
    // via popup?
void until (state_t *state) {}





/***************
   Popup window
 ***************/


// TODO:
static void open_popup_window (state_t *state) {}



// TODO:
    // add "builds" section to config file
    // build_t linked list ({ name, build_str });
    // popup window
void pwin_builds (state_t *state) {}



void pwin_layouts (state_t *state) {}



/*********
  Windows
 *********/


void win_assembly (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_assembly (state);
            break;
    }
}



void win_breakpoints (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_breakpoints (state);
            break;
    }
}



void win_local_vars (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_local_vars (state);
            break;
    }
}



void win_prog_output (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_prog_output (state);
            break;
    }
}



void win_prompt (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_prompt (state);
            break;
    }
}



void win_registers (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_registers (state);
            break;
    }
}



void win_src_file (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_src_file (state);
            break;
    }
}



void win_watches (state_t *state)
{
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB):
            gdb_win_watches (state);
            break;
    }
}



/********************
  Update window data
 ********************/



