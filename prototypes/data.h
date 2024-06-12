#ifndef DATA_H
#define DATA_H

#include <ncurses.h>
#include <semaphore.h>


#define DEBUG_SEMAPHORE  "/debug_semaphore"


typedef struct debug_state {

    int    debugger;
    int    input_pipe;
    int    output_pipe;
    char  *prog_path;
    char  *break_point;
    char  *out_done_str;
    char  *exit_str;
    FILE  *out_file_ptr;
    char   output_indicator;

} debug_state_t;


typedef struct window {

    WINDOW            *win;
    char               key;
    bool               selected;

    int                win_rows;                   
    int                win_cols;                   
    int                win_y;                      
    int                win_x;                      
    int                win_border [8];
    int                win_mid_line;
    struct window     *win_next;           

    FILE              *file_ptr;
    char               file_path [256];
    int                file_first_char;
    int                file_rows;
    int                file_max_cols;
    int                file_min_mid;
    int                file_max_mid;
    unsigned long int *file_offsets;

} window_t;

typedef struct data {

    window_t *window;
    debug_state_t *debug_state;

} data_t;


#endif
