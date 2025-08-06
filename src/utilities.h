#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "parse_config_file.h"
#include "render_layout.h"



/*
    Log formatted string to DEBUG_OUT_FILE for debugging inside ncurses
    --------
    - Same usage as printf()
*/
void logd (const char *formatted_string, ...);


/*
    Initial configuration
    ---------
    - CLI flags
    - Set state pointer
    - Set signals
    - Initialize ncurses
*/
int initial_configure (int argc, char *argv[], state_t *state);



/*
    Free all ncurses windows in current layout except state->header
*/
int free_nc_window_data (state_t *state);


/*
    Clean up before exiting program
*/
enum {
    PROG_EXIT,      // Non-error exit, e.g. signal interrupt
    PROG_ERROR      // Program error
};
void clean_up (int type);





/*
    Set, unset Ncurses attribute with variable 
    -----------
    - Must normally set, unset attributes with wattron/off using constant or macro
    - This function allows you to use integer variables (predefined in functions)
*/
int set_nc_attribute   (WINDOW *nc_window, int attribute_value);
int unset_nc_attribute (WINDOW *nc_window, int);



/*
    Find string in Ncurses window
    ----------
    - Sets y,x variables to first character's coordinates
    - returns true if string found, false otherwise
*/
bool find_window_string (WINDOW *nc_window, char *string, int *y, int *x);



/*
    Compare file's previously-saved update time with its current time
    -------
    - Returns true if updated, false otherwise
*/
bool file_was_updated (time_t prev_file_mtime, char *file_path);



/*
    Set pointer to state_t in utilities.c
*/
void set_state_ptr (state_t *state);



/*
    Copy single character to plugin window buffer
    -----------
    state->plugins[x]->win->buff_data

    Uses: 
        ->buff_pos
        ->buff_len
        ->times_doubled
*/
void cp_wchar (buff_data_t *dest_buff_data, char ch);



/*
    Create buffer from file
    -------
    - Returns pointer to buffer or NULL if failed
    - Buffer must be freed
*/
char* create_buff_from_file (char *path);



#ifdef DEV

/*
    Log formatted string to DEBUG_OUT_FILE
    --------
    - Same usage as printf()
*/
void logd (const char *formatted_string, ...);



/*
    Log current state_t values to DEBUG_OUT_FILE
*/
void log_state_t (state_t *state);

#endif


#endif
