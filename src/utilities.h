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
    Send debugger command only
    -------
    - Must end with '\n'
*/
int send_command (state_t *state, char *command);



/*
    Send debugger command string with start, end markers; parse output
    -------
    - Must end with '\n'
*/
int send_command_mp (state_t *state, char *command);



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
    Set pointer to state in utilities.c for persisting data on clean_up()
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
    Copy character into src_file_data_t buffers
    --------
    state->plugins[Src]->win->src_file_data  (->path, ->addr, ->func)

    Uses:
        ->path_pos
        ->path_len
        ->addr_ ...
*/
enum { PATH, ADDR, FUNC };      // type

void cp_fchar (src_file_data_t *dest_file_data, char ch, int type);



/*
    Copy character into debugger buffer
    ---------
    state->debugger

    Uses:
        ->format_pos
        ->format_len
        ->format_times_doubled
        ->data_pos
        ->data_ ..l
*/
enum { FORMAT_BUF, DATA_BUF, CLI_BUF, PROGRAM_BUF, ASYNC_BUF };     // buff_index

void cp_dchar (debugger_t *debugger, char ch, int buff_index);



/*
  Print formatted error messages
  ----------
  - For error propagation that will ultimately exit the program
    - Runs clean_up()

    Message, return FAIL
  
        pfemr ("Unknown character \"%c\"", ch);
    
        ERROR: src_file.c : func() : 10
               Unknown character "c"

    Message, exit program

        pfeme ("Unknown character \"%c\"", ch);
  
    Message

        pfem ("Failed to allocate buffer");
        return NULL;
  
    Multiple messages, return FAIL
  
        pfem ("Unknown character \"%c\"", ch);
        pem  ("Check README.md for more details");
        pem  ("Check the website for video demos");
        pemr ("Returning FAIL...");
    
        ERROR: src_file.c : func() : 10
               Unknown character "c"
               Check README.md for more details
               See a physician if your symptoms persist
               Returning FAIL...

    Multiple messages then exit program
        
        ...
        peme ("Exiting...");

    errno message, return FAIL

        if ((buff = malloc (4096)) == NULL) {
            pfemr ("malloc error: %s", strerror (errno));
        }
*/

// Print formatted error message
#define pfem(...) do { \
    clean_up();\
\
    fprintf (stderr, "\n\
\033[1;31m%s\033[1;0m \
\033[1;32m%s\033[1;0m() : \
\033[1;36m%s\033[1;0m : \
\033[1;33m%d\033[1;0m\n\
       ", \
    "ERROR:", __func__, __FILE__, __LINE__);\
\
    fprintf (stderr, __VA_ARGS__);\
    fprintf (stderr, "\n"); \
} while (0)

// Print formatted error message, return
#define pfemr(...) ({ \
    pfem(__VA_ARGS__);\
    return FAIL;\
})

// Print formatted error message, exit
#define pfeme(...) do { \
    pfem(__VA_ARGS__); \
    fprintf (stderr, "\n");\
    exit (EXIT_FAILURE); \
} while (0)

// Print error message
#define pem(...) do { \
    fprintf (stderr, "       "); \
    fprintf (stderr, __VA_ARGS__); \
    fprintf (stderr, "\n"); \
} while (0)

// Print error message, return
#define pemr(...) ({\
    pem(__VA_ARGS__);\
    return FAIL;\
})

// Print error message, exit
#define peme(...) do { \
    pem(__VA_ARGS__);\
    fprintf (stderr, "\n");\
    exit (EXIT_FAILURE);\
} while (0)



#endif

