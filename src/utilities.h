
/*
    Misc utilities
    -------------

    arrlen()                - get length of array
    pfem()                  - print file, error info
    pfemo()                 - print       error info
    print_int_matrix()      - print matrix values (ncurses)

*/

#ifndef UTILITIES_H
#define UTILITIES_H


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "parse_config_file.h"
#include "render_layout.h"

void  clean_up              (void);
char *create_path           (int, ...);
void  send_command          (state_t*, int, ...);
int   getkey                (void);
char* create_path           (int, ...);
void  write_to_pipe         (int, char**);
void  set_nc_attribute      (WINDOW*, int);
void  unset_nc_attribute    (WINDOW*, int);



/****************
  Error handling
 ****************

Single message, exit:

    pfeme ("Unknown character \"%c\" \n", ch);

    ERROR: src_file.c : func () :10
           Unknown character "c"

Multiple messages, exit;

    pfem ("Unknown character \"%c\" \n", ch);
    pem  ("Check README.md for more details");
    peme ("Exiting...");

    ERROR: src_file.c : func () :10
           Unknown character "c"
           Check README.md for more details
           Exiting...
*/

// Print formatted error message
#define pfem(...) do { \
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
    clean_up(); \
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
    clean_up(); \
    pem(__VA_ARGS__); \
    exit (EXIT_FAILURE); \
} while (0)



#endif
