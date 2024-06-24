
/*
    Interface for plugin and other debugger-specific functions
*/

#include <string.h>

#include "gdb_plugins.h"
#include "../data.h"
#include "../window_updates/_insert_output_marker.h"
#include "../render_window_data.h"
#include "../window_updates/_win_updates_interface.h"

char *plugin_path;


/*
    Placeholder function at plugin_function[0]  (run_plugin.c)
    -----------
    - Allows 0 to stand for "unassigned" in key_function_index[]  (data.h)
*/
void empty_func (state_t *state) { 
    (void) state; 
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
    (void) state;
    render_window_data (NULL, state, NULL, WINDOW_UNSELECT);
}



void scroll_down (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, state, KEY_DOWN, WINDOW_DATA);
    }
}



void scroll_up (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, state, KEY_UP, WINDOW_DATA);
    }
}



void scroll_left (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, state, KEY_LEFT, WINDOW_DATA);
    }
}



void scroll_right (state_t *state)
{
    if (state->curr_window) {
        render_window_data (state->curr_window, state, KEY_RIGHT, WINDOW_DATA);
    }
}



void run_other_win_key (int key, state_t *state)
{
    window_t *win = state->curr_window;
    win_keys_t *k = state->win_keys;

    // Use k->back in window while loop as exit condition

    if (key == k->scroll_up) {
        render_window_data (state->curr_window, state, KEY_UP, WINDOW_DATA);
    } 
    else if (key == k->scroll_down) {
        render_window_data (state->curr_window, state, KEY_DOWN, WINDOW_DATA);
    } 
    else if (key == k->scroll_left) {
        render_window_data (state->curr_window, state, KEY_LEFT, WINDOW_DATA);
    } 
    else if (key == k->scroll_right) {
        render_window_data (state->curr_window, state, KEY_RIGHT, WINDOW_DATA);
    } 
    else if (key == k->quit) {
        quit (state); 
    } 
    else {
        // TODO: pg up/down, home, end
        switch (key) {
            case KEY_UP:
                render_window_data (state->curr_window, state, KEY_UP, WINDOW_DATA);
                break;
            case KEY_DOWN:
                render_window_data (state->curr_window, state, KEY_DOWN, WINDOW_DATA);
                break;
            case KEY_RIGHT:
                render_window_data (state->curr_window, state, KEY_RIGHT, WINDOW_DATA);
                break;
            case KEY_LEFT:
                render_window_data (state->curr_window, state, KEY_LEFT, WINDOW_DATA);
                break;
        }
    }
}




/*******************
  Debugger commands
 *******************/


void cont (state_t *state)
{
    insert_output_start_marker ("Prm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_continue (state); break;
    }
    insert_output_end_marker ("Prm", state);
}



void finish (state_t *state)
{
    insert_output_start_marker ("Prm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_finish (state); break;
    }
    insert_output_end_marker ("Prm", state);
}



void kill_prog (state_t *state)
{
    insert_output_start_marker ("Prm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_kill (state); break;
    }
    insert_output_end_marker ("Prm", state);
}



void next (state_t *state)
{
    insert_output_start_marker ("Prm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_next (state); break;
    }
    insert_output_end_marker ("Prm", state);
}



void run (state_t *state)
{
    insert_output_start_marker ("Prm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_run (state); break;
    }
    insert_output_end_marker ("Prm", state);
        //
    update_local_vars (state);
}



void step (state_t *state)
{
    insert_output_start_marker ("Prm", state);
    switch (state->debug_state->debugger) {
        case (DEBUGGER_GDB): gdb_step (state); break;
    }
    insert_output_end_marker ("Prm", state);
}



// TODO: 
    // add line int to command
    // via popup?
void until (state_t *state) {
    (void) state;
}





/****************
   Popup windows
 ****************/


// TODO:
void open_popup_window (state_t *state)
{
    (void) state;
}



// TODO:
    // add "builds" section to config file
    // build_t linked list ({ name, build_str });
    // popup window
void pwin_builds (state_t *state) {
    (void) state;
}



// TODO:
void pwin_layouts (state_t *state)
{
    (void) state;
}



// TODO:
void pwin_source_files (state_t *state)
{
    (void) state;
}



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



