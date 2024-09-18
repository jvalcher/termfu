#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parse_config_file.h"
#include "render_layout.h"



/*
    Log formatted string to DEBUG_OUT_FILE for debugging inside Ncurses
    --------
    - Same usage as printf()
*/
void logd (const char *formatted_string, ...);



/*
    Clean up before exiting program
*/
void  clean_up  (void);



/*
    Concatenate variable number of strings
    -------
    - Returns pointer to created string
    - Must free after use
*/
char  *concatenate_strings  (int num_strings, ...);



/*
    Send debugger command string
*/
void  send_command  (state_t *state, char *command_string);



/*
    Send debugger command string with start, end markers and parse output
*/
void send_command_mp (state_t *state, char *command);



/*
    Set, unset Ncurses attribute with variable 
    -----------
    - Must normally set, unset attributes with wattron/off using constant or macro
    - This function allows you to use integer variables (predefined in functions)
*/
void  set_nc_attribute    (WINDOW *nc_window, int attribute_value);
void  unset_nc_attribute  (WINDOW *nc_window, int);



/*
    Find string in Ncurses window
    ----------
    - Sets y,x variables to first character's coordinates
    - returns true if string found, false otherwise
*/
bool  find_window_string  (WINDOW *nc_window, char *string, int *y, int *x);



/*
    Copy single character to destination buff_data_t->buff (circular)
*/
void cp_char (buff_data_t *dest_buff_data, char ch);



/*
    Copy single char to  state->plugins[Src]->win->file_data_t->path, ->addr, ->func  buffers
    --------
    type: PATH, ADDR, FUNC
*/
enum { PATH, ADDR, FUNC };
    //
void cp_fchar (file_data_t *dest_file_data, char ch, int type);



/*
    Compare file's previously-saved update time with its current time
    -------
    - Returns true if updated, false otherwise
*/
bool file_was_updated (time_t file_mtime, char *file_path);



/****************
  Formatted error messages
 ****************
  Single message, exit:
  
      pfeme ("Unknown character \"%c\" \n", ch);
  
      ERROR: src_file.c : func() : 10
             Unknown character "c"
  
  Multiple messages, exit;
  
      pfem ("Unknown character \"%c\" \n", ch);
      pem  ("Check README.md for more details"); 
      peme ("Exiting...");
  
      ERROR: src_file.c : func() : 10
             Unknown character "c"
             Check README.md for more details
             Exiting...
*/

// Print formatted error message
#define pfem(...) do { \
    clean_up(); \
\
    fprintf (stderr, "\
\033[1;31m%s\033[1;0m \
\033[1;32m%s\033[1;0m() : \
\033[1;36m%s\033[1;0m : \
\033[1;33m%d\033[1;0m\n       ", \
    "ERROR:", __func__, __FILE__, __LINE__); \
\
    fprintf (stderr, __VA_ARGS__); \
} while (0)


// Print formatted error message, exit
#define pfeme(...) do { \
    pfem(__VA_ARGS__); \
    exit (EXIT_FAILURE); \
} while (0)


// Print error message
#define pem(...) do { \
    fprintf (stderr, "       "); \
    fprintf (stderr, __VA_ARGS__); \
} while (0)


// Print error message, exit
#define peme(...) do { \
    pem(__VA_ARGS__); \
    exit (EXIT_FAILURE); \
} while (0)



#endif
