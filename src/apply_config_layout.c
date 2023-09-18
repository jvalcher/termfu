
/*
    Parse config file layouts
    --------------
    Add window data from config_file_data struct (get_config.h)
    to layouts struct
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "apply_config.h"
#include "apply_config_layout.h"
#include "utilities.h"


void apply_config_layout (
        int i,
        int scr_rows,
        int scr_cols,
        struct config_file_data* data, 
        struct layouts *layouts)
{
    // calculate window segment ratio
    //
    //    * * *   y_ratio == 2
    //    * * *   x_ratio == 3
    //
    int j;
    bool  x_count     = true;
    int   y_ratio     = 0;
    int   x_ratio     = 0;
    for (j = 0; j < strlen (data->categ_values [i]); j++) {
        if (data->categ_values [i][j] == 'n') {
            y_ratio += 1;
            x_count = false;
        }
        if (x_count) {
            x_ratio += 1;
        }
    }

    // create layout matrix
    //
    //      sssb
    //      sssw
    //      cccr 
    //
    int col = 0;
    int row = 0;
    char layout_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};
    for (j = 0; j < strlen (data->categ_values[i]); j++) {
        layout_matrix [row][col] = data->categ_values [i][j];
        if (col < x_ratio - 1) {
            col += 1;
        } else {
            col  = 0;
            row += 1;
            j   += 1;     // skip 'n'
        }
    }

    // create (un)used segments matrix
    //
    //         1110
    //         1110
    //         0000
    //
    int used_segm_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};

    // calculate rows, columns in each segment
        // rows
    int rem_val;
    int y_segm_rows [MAX_Y_SEGMENTS] = {0};               // rows per segment array
    int y_segm_len = scr_rows / y_ratio;                  // floor row amount per segment
    int y_remainder = scr_rows - (y_segm_len * y_ratio);  // remainder
    for (j = 0; j < y_ratio; j++) {                       // distribute remainder
        rem_val = (y_remainder-- > 0) ? 1 : 0;
        y_segm_rows [j] = y_segm_len + rem_val;
    }
        // columns
    int x_segm_cols [MAX_X_SEGMENTS] = {0};
    int x_segm_len = scr_cols / x_ratio;
    int x_remainder = scr_cols - (x_segm_len * x_ratio);
    for (j = 0; j < x_ratio; j++) {
        rem_val = (x_remainder-- > 0) ? 1 : 0;
        x_segm_cols [j] = x_segm_len + rem_val;
    }
    
    struct window *curr_window = NULL;

        // window symbol -> title string array (get_config.h)
    char *layout_titles [MAX_WINDOW_TYPES] = {0};
    layout_titles ['a'] = "Assembly";
    layout_titles ['b'] = "Breakpoints";
    layout_titles ['c'] = "Commands";
    layout_titles ['l'] = "Local vars";
    layout_titles ['r'] = "Registers";
    layout_titles ['s'] = "Source";
    layout_titles ['t'] = "Status";
    layout_titles ['w'] = "Watches";

        // loop through layout_matrix segments
    for (int y = 0; y < y_ratio - 1; y++) {
        for (int x = 0; x < x_ratio - 1; x++) {

            // if unused segment
            if (used_segm_matrix [x][y] == 0) {

                int ch = layout_matrix [x][y];
                layouts->num_windows [i] += 1;

                // create window struct
                curr_window = (struct window*) malloc (sizeof (struct window));
                if (curr_window == NULL) {
                    endwin ();
                    pfem ("Unable to create window struct");
                    exit (EXIT_FAILURE);
                }

                // set head
                if (layouts->num_windows [i] == 1) {
                    layouts->windows [i] = curr_window;
                }

                // add title
                if (layout_titles [ch] != 0) {
                    strncpy (curr_window->title, 
                             layout_titles [ch], 
                             sizeof (char) * MAX_TITLE_LEN);
                } else {
                    char ch_err_mes[] = "Unknown window symbol (_)";
                    ch_err_mes [strlen (ch_err_mes) - 2] = ch;
                    endwin ();
                    pfem (ch_err_mes);
                    exit (EXIT_FAILURE);
                }

                // calculate top left y,x coordinate by
                // adding rows/cols of preceding segment(s)
                curr_window->y = 0;
                for (j = y - 1; j >= 0; j--) {
                    curr_window->y += y_segm_rows [j];
                }
                curr_window->x = 0;
                for (j = x - 1; j >= 0; j--) {
                    curr_window->x += x_segm_cols [j];
                }

                // TODO: 
                // remove shared border cols/rows
                // add 1 at end

                // calculate width, height 
                //  
                //         ┌──aa
                //         │ssaa
                //         │sscc
                //         bbwwr
                //
                    // width in rows
                curr_window->cols = x_segm_cols [x];
                int xi;
                for (xi = x+1; xi < x_ratio; xi++) {
                    if (ch == layout_matrix [y][xi]) {
                        curr_window->cols += x_segm_cols [xi];
                    } else {
                        break;
                    }
                }

                    // height in columns
                curr_window->rows = y_segm_rows [y];
                int yi;
                for (yi = y+1; yi < y_ratio; yi++) {
                    if (ch == layout_matrix [yi][x]) {
                        curr_window->rows += y_segm_rows [yi];
                    } else {
                        break;
                    }
                }

                // mark used segments in used_segm_matrix
                for (j = y; j < yi; j++) {
                    for (int k = x; k < xi; k++) {
                        used_segm_matrix [j][k] = 1;
                    }
                }

            }
        }
    }
    curr_window = NULL;
}
