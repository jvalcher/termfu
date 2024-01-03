
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

#include "parse_config.h"
#include "render_layout.h"



/*
    Get length of array
*/
#define arrlen(a) (sizeof(a) / sizeof *(a))


/*
    Print formatted message to stderr
    -----------

    pfem ("Unknown character \"%c\" \n", ch);
    -->

        ERROR: src_file.c : func () : 10
               Unknown character "t"

    ANSI color escape codes:

        white   -> \033[1;37m
        cyan    -> \033[1;36m
        green   -> \033[1;32m
        blue    -> \033[1;34m
        red     -> \033[1;31m
        yellow  -> \033[1;33m

        end     -> \033[1;0m
*/
#define pfem(...) do { \
\
    fprintf (stderr, "\
\033[1;31m%s\033[1;0m \
\033[1;32m%s\033[1;0m() : \
\033[1;36m%s\033[1;0m : \
\033[1;33m%d\033[1;0m\n       ", \
    "ERROR:", \
    __func__, \
    __FILE__, \
    __LINE__); \
\
    fprintf (stderr, __VA_ARGS__); \
} while (0)



/*  
    Print formatted error message only 
    -------------

    pfem ("Unknown character \"%c\" \n", ch);

    pfem  ("My error message");
    pfemo ("var1 == 5");

    ERROR: src_file.c : func () :10
           My error message
           var1 == 5
*/
#define pfemo(...) do { \
    fprintf (stderr, "       "); \
    fprintf (stderr, __VA_ARGS__); \
} while (0)



/*
    Print integer matrix [y][x] values
    ---------
    start_row   - row to start printing (ncurses)
*/
void print_int_matrix ( char *label,
                        int start_row, 
                        int matrix [MAX_ROW_SEGMENTS][MAX_COL_SEGMENTS], 
                        int y, 
                        int x);



/*
    Print bold, colored Ncurses string with mvwprintw()
    ---------
    Takes int color variable instead of integer constant or macro

    Uses color pairs in src/data.h

    Usage:

        mv_print_title (GREEN_BLACK, win, 3, 2, "Layout: %s", layout_str);
*/
void mv_print_title (int     color, 
                     WINDOW *win,
                     int     row,
                     int     col, 
                     char   *str);


/*
    Set, unset color using int variable
    -----------
    wattron/off() require the use of integer constants or macros
*/
void set_bold_color (WINDOW* win, int color);
void unset_bold_color (WINDOW* win, int color);



#endif
