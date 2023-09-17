
/*
    Misc utilities
*/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    Print formatted message to stderr
    -----------

    pfem ("My error message");
    -->
    ERROR: src_file.c : 10 : My error message

    ANSI color escape codes:

        white   -> \033[1;37m
        cyan    -> \033[1;36m
        green   -> \033[1;32m
        blue    -> \033[1;34m
        red     -> \033[1;31m
        yellow  -> \033[1;33m

        end     -> \033[1;0m
*/
#define pfem(message) do { \
\
fprintf (stderr, " \
\033[1;31m%s\033[1;0m \
\033[1;36m%s\033[1;0m : \
\033[1;32m%s\033[1;0m() : \
\033[1;37m%s\033[1;0m \n", \
    "ERROR:",   \
    __FILE__,   \
    __func__,   \
    (message)); \
\
} while (0)


/*  
    Print formatted error message only 
    -------------
    Preserve spacing of ERROR, __FILE__, __func__

    pfem  ("My error message");
    pfemo ("var1 == 5");
    pfemo ("var2 == 10");
    -->
    ERROR: src_file.c : 10 : My error message
                           : var1 == 5
                           : var2 == 10
*/
#define pfemo(message) do { \
\
int err_len         = strlen("ERROR:"); \
int file_len        = strlen(__FILE__); \
int func_len        = strlen(__func__); \
int other_char_len  = 7; \
int num_spaces = err_len + file_len + func_len + other_char_len; \
\
char *space_str = (char *) malloc (num_spaces + 1); \
memset (space_str, ' ', num_spaces); \
space_str [num_spaces] = '\0'; \
fprintf (stderr, "%s : %s\n", space_str, (message)); \
free (space_str); \
\
} while (0)




#endif
