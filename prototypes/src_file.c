/*
   Render Ncurses layout (not including window data)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>
#include <stdbool.h>
#include <ncurses.h>

#ifdef DEBUG
#include <sys/ioctl.h>
#include <unistd.h>
#endif

#include "data.h"
#include "render_layout.h"
#include "utilities.h"

static void      allocate_windows      (state_t *state);
static void      free_window_data      (state_t*);
static layout_t *get_label_layout      (char*, layout_t*);
static void      calculate_layout      (layout_t*, state_t*);

#ifndef DEBUG
    static void      render_header         (layout_t*, state_t*);
    static void      render_windows        (state_t*);
    static void      render_header_titles  (layout_t*, state_t*);
    static WINDOW   *allocate_window       (int, int, int, int);
    static void      render_window         (window_t*);
    static void      render_window_titles  (state_t*);
    static void      fix_corners           (state_t*);
    static int       fix_corner_char       (int, int);
#endif

#ifdef DEBUG
    static void      print_layout_info     (layout_t*);
    static void      get_rows_cols         (int*, int*);
#endif

bool  window_t_arr_is_not_allocated = true;
int   scr_rows;
int   scr_cols;
int   header_offset;



void
render_layout (char     *label,
               state_t  *state)
{
    layout_t *layout;

    if (window_t_arr_is_not_allocated) {
        allocate_windows (state);
        window_t_arr_is_not_allocated = false;
    } else {
        free_window_data (state);
    }

    layout = get_label_layout (label, state->layouts);

    calculate_layout (layout, state);

#ifdef DEBUG

    print_layout_info (layout);

#endif

#ifndef DEBUG

    render_header  (layout, state);

    render_windows (state);

#endif
}
