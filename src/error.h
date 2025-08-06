/*
    Formatted error messages
    ----------
    - Current file, function info
    - Error propagation

    - Usage:
  
        pfemr ("Unknown character \"%c\"", ch);
    
    - Output:
    
          src_file.c : func():10 :: Unknown character "a"
          ...

    - Functions

        pfem            - print message
        pfemr           - print message, return FAIL
        pfemn           - print message, return NULL
        pfeme           - print message, exit
        pfem_errno      - print "message (errno message)"
        pfemr_errno     - print "message (errno message)", return FAIL
        pfemn_errno     - print "message (errno message)", return NULL
        pfeme_errno     - print "message (errno message)", exit
*/

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

#define FAIL  0
#define A_OK  1

#define R       "\033[1;0m"         // reset to default
#define RED     "\033[1;31m"
#define CYAN    "\033[1;36m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define PURPLE  "\033[1;35m"

/*
    Base formatted error message function
    -------------
    - Calls clean_up() first
*/
#define pfem_impl(...)                                                                   \
do {                                                                                     \
    fprintf (stderr,                                                                     \
        "  " CYAN "%s" R ":" YELLOW "%d" R "  --  " GREEN "%s" PURPLE "()" R "\n      ", \
        __FILE__, __LINE__, __func__);                                                   \
    fprintf (stderr, __VA_ARGS__);                                                       \
} while (0)

/*
    Print formatted message
*/

// Message only
#define pfem(...)               \
do {                            \
    pfem_impl(__VA_ARGS__);     \
    fprintf (stderr, "\n");     \
} while (0)                     \

// Return FAIL
#define pfemr(...)          \
do {                        \
    pfem (__VA_ARGS__);     \
    return FAIL;            \
} while (0)

// Return NULL
#define pfemn(...)          \
do {                        \
    pfem (__VA_ARGS__);     \
    return NULL;            \
} while (0)

// Exit
#define pfeme(...)          \
do {                        \
    pfem (__VA_ARGS__);     \
    exit (EXIT_FAILURE);    \
} while (0)

/*
    Print formatted message + errno message
*/

// Message only
#define pfem_errno(...)                                             \
do {                                                                \
    pfem_impl (__VA_ARGS__);                                        \
    fprintf (stderr, "  (" PURPLE "%s" R ")\n", strerror(errno));   \
} while (0)

// Return FAIL
#define pfemr_errno(...)        \
do {                            \
    pfem_errno (__VA_ARGS__);   \
    return FAIL;                \
} while (0)

// Return NULL
#define pfemn_errno(...)        \
do {                            \
    pfem_errno (__VA_ARGS__);   \
    return NULL;                \
} while (0)

// Exit
#define pfeme_errno(...)        \
do {                            \
    pfem_errno (__VA_ARGS__);   \
    exit (EXIT_FAILURE);        \
} while (0)



#endif
