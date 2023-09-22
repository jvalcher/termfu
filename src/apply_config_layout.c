#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "get_config.h"
#include "apply_config.h"
#include "apply_config_layout.h"
#include "utilities.h"


/*
    Parse config file layouts
    --------------
    Add window size, position data from config_file_data 
    struct (get_config.h) to layouts struct
*/
void apply_config_layout (int di,
                          int li,
                          struct config_file_data *data, 
                          struct layouts *layouts,
                          int scr_rows,
                          int scr_cols)
{
    // add layout name
    strcpy (layouts->labels [li], data->categ_labels [di]);
    printf("%s \n-------\n", layouts->labels [li]);

    // calculate window segment ratio
    //
    //    * * *   y_ratio == 2
    //    * * *   x_ratio == 3
    //
    int j;
    bool  x_count     = true;
    int   y_ratio     = 0;
    int   x_ratio     = 0;
    for (j = 0; j < strlen (data->categ_values [di]); j++) {
        if (data->categ_values [di][j] == 'n') {     // add new row
            y_ratio += 1;
            x_count = false;
        }
        if (x_count) {          // add new column
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
    for (j = 0; j < strlen (data->categ_values[di]); j++) {
        layout_matrix [row][col] = data->categ_values [di][j];
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

    // calculate segment sizes
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
    
    // create pointers for linking window objects
    struct window *curr_window = NULL;
    struct window *prev_window = NULL;

    // create window symbol -> title string array (get_config.h)
    /*
    char *layout_titles [MAX_WINDOW_TYPES] = {0};
    layout_titles ['a'] = "Assembly";
    layout_titles ['b'] = "Breakpoints";
    layout_titles ['c'] = "Commands";
    layout_titles ['l'] = "Local vars";
    layout_titles ['r'] = "Registers";
    layout_titles ['s'] = "Source";
    layout_titles ['t'] = "Status";
    layout_titles ['w'] = "Watches";
    */



    // loop through layout_matrix segments
    for (int y = 0; y < y_ratio; y++) {
        for (int x = 0; x < x_ratio; x++) {

            // if unused segment
            if (used_segm_matrix [y][x] == 0) {

                // TODO: check symbol (hash table...)
                if (layout_titles [ch] != 0) {
                    ;
                } else {
                    char ch_err_mes[] = "Unknown window symbol (_)";
                    ch_err_mes [strlen (ch_err_mes) - 2] = ch;
                    endwin ();
                    pfem (ch_err_mes);
                    exit (EXIT_FAILURE);
                }

                // increment number of windows
                layouts->num_windows [li] += 1;

                // get window symbol character
                char ch = layout_matrix [y][x];

                printf("%c\n", ch);

                // create window
                struct window *window = (struct window*) malloc (sizeof (struct window));
                if (window == NULL) {
                    endwin ();
                    pfem ("Unable to create window struct");
                    exit (EXIT_FAILURE);
                }
                curr_window = window;
                curr_window->next = NULL;

                // set head or link previous window
                if (layouts->num_windows [li] == 1) {
                    layouts->windows [li] = window;     // set head window
                } else {
                    prev_window->next = curr_window;    // else, link to previous window
                }

                // add symbol
                curr_window->symbol = ch;

                // calculate top left y,x coordinate by
                // adding rows/cols of preceding segment(s)
                curr_window->y = 0;
                for (j = y - 1; j >= 0; j--) {
                    curr_window->y += y_segm_rows [j];  // add total rows in preceding segment
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

                // mark used segments
                for (j = y; j < yi; j++) {
                    for (int k = x; k < xi; k++) {
                        used_segm_matrix [j][k] = 1;
                    }
                }

                // print values (debug)
                printf("%c %s %d %d %d %d\n", 
                    curr_window->symbol,
                    curr_window->title,
                    curr_window->rows,
                    curr_window->cols,
                    curr_window->y,
                    curr_window->x);

                prev_window = curr_window;

            }   // if unused segment

        }
    }   // outer for loop

}
