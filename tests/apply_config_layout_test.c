#include <stdio.h>
#include <ncurses.h>

#include "../src/render_screen.h"


/*
    Test apply_config_layout.c
*/
int main (void) 
{
    int i = 2;
    int rows = 0;
    int cols = 0;

    // calculate current terminal pane dimensions
    initscr();
    int scr_cols = getmaxx (stdscr);
    int scr_rows = getmaxy (stdscr) - 2;   // minus program title bar
    endwin();

    // create structs

        // config_file_data
    struct config_file_data data = {

        .num_configs      = 2,

        .categories   [0] = "layout",
        .categ_labels [0] = "test_layout_1",
        .categ_values [0] = "aaanbbbncccn",

        .categories   [1] = "layout",
        .categ_labels [1] = "test_layout_2",
        .categ_values [1] = "llnrrnssn"
    };

        // layouts
    struct layouts layouts = {0};
    layouts.num_layouts = 0;

    // parse layout for
        // config 1
    layouts.num_layouts += 1;
    int di = 0;
    int li = 0;
    apply_config_layout (di, li, &data, &layouts, scr_rows, scr_cols);
        // config 2
    layouts.num_layouts += 1;
    di += 1;
    li += 1;
    apply_config_layout (di, li, &data, &layouts, scr_rows, scr_cols);

    /*
    // print results
    struct window *window;
    printf ("num_layouts: %d\nscreen size: cols=%d rows=%d\n\n", layouts.num_layouts, scr_cols, scr_rows);

    // print layout info
    for (int l = 0; l < layouts.num_layouts; l++) {

        // label, number of windows
        printf ("layout: %s\nnum_windows: %d\n-------\n", 
                    layouts.labels[l],
                    layouts.num_windows[l]);

        // windows
        window = layouts.windows[l];
        for (int w = 0; window != NULL; w++) {
            printf ("symbol: %c\ntitle: %s\nrows: %d\ncols: %d\ny: %d\nx: %d\nnext: %p\n\n", 
                        window->symbol,
                        window->title,
                        window->rows,
                        window->cols,
                        window->y,
                        window->x,
                        window->next);

            window = layouts.windows[l]->next;
        }

    }
    */

    return 0;
}




