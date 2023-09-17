
/*
   Render Ncurses layouts
*/

#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "parse_config.h"
#include "get_config.h"

static struct window *create_window_struct ();


/*
    Parse config_file_data struct (get_config.h) settings
    Pass resulting data into apply_config.h structs
    Add to respective array
*/
void parse_config ()
{
    /*
        Add CONFIG_FILE data to config_file_data struct  (get_config.h)
    */
    struct config_file_data data = {0};
    get_config_file_data (&data);


    /*
        Create config data structures  (parse_config.h)
    */
    struct layouts layouts;
    layouts.num_layouts = 0;

    /*
       Parse config file data
       Add resulting data to structs defined in parse_config.h
       ------------
       - 'i' iterates through layouts data members
    */
    for (int i = 0; i < data.num_configs; i++) {

        // if layout
        if (strcmp (data.categories[i], "layout") == 0) {

            // increment number of layouts
            layouts.num_layouts += 1;

            // calculate window segment ratio
            //
            //      ssaaw       y_ratio == 3
            //      ssaab       x_ratio == 5 
            //      cccrr 
            //
            int j;
            bool  x_count     = true;
            int   y_ratio     = 0;
            int   x_ratio     = 0;
            for (j = 0; j < strlen(data.categ_values[i]); j++) {
                if (data.categ_values[i][j] == 'n') {
                    y_ratio += 1;
                    x_count = false;
                }
                if (x_count) {
                    x_ratio += 1;
                }
            }

            //  create layout matrix
            //
            //      sssb
            //      sssw
            //      cccr 
            //
            int col = 0;
            int row = 0;
            char layout_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};
            for (j = 0; j < strlen(data.categ_values[i]); j++) {
                layout_matrix [col][row] = data.categ_values [i][j];
                if (col < x_ratio) {
                    col += 1;
                } else {
                    col  = 0;
                    row += 1;
                    j   += 1;     // skip 'n'
                }
            }

            // create (un)used segments matrix
            //
            //      1110
            //      1110
            //      0000
            //
            int used_segm_matrix [MAX_Y_SEGMENTS][MAX_X_SEGMENTS] = {0};

            // calculate current terminal's dimensions
            int screen_cols = getmaxx (stdscr);
            int screen_rows = getmaxy (stdscr) - 2;   // minus program title bar

            // calculate rows in each segment
            int y_segm_rows [MAX_Y_SEGMENTS] = {0};
            int y_segm_len = screen_rows / y_ratio;
            int y_remainder = screen_rows - (y_segm_len * y_ratio);
            int rem_val;
            for (j = 0; j < y_ratio; j++) {
                rem_val = (y_remainder-- > 0) ? 1 : 0;
                y_segm_rows [j] = y_segm_len + rem_val;
            }
            
            // calculate columns in each segment
            int x_segm_cols [MAX_X_SEGMENTS] = {0};
            int x_segm_len = screen_cols / x_ratio;
            int x_remainder = screen_cols - (x_segm_len * x_ratio);
            for (j = 0; j < x_ratio; j++) {
                rem_val = (x_remainder-- > 0) ? 1 : 0;
                x_segm_cols [j] = x_segm_len + rem_val;
            }
            
            // add windows
            int win_i = 0;
            layouts.windows [win_i] = 0;
            struct window *window;
            for (int y = 0; y < y_ratio - 1; y++) {
                for (int x = 0; x < x_ratio - 1; x++) {

                    // if unused segment
                    if (used_segm_matrix [x][y] == 0) {

                        layouts.windows [win_i] += 1;

                        // malloc new window struct
                        window = create_window_struct ();

                        // add window title
                        if (layout_titles [ch] != '\0') {
                            strncpy ( window.win_title, 
                                      layout_titles [ch], 
                                      sizeof(layout_titles [ch]) );
                        } else {
                            char ch_err_mes[] = "Unknown window symbol (_)";
                            ch_err_mes [strlen(ch_err_mes) - 2] = ch;
                            pfem (ch_err_mes);
                            exit (EXIT_FAILURE);
                        }

                        // TODO: 
                        // add top, left coordinates
                        // Need to use segment lengths

                        // TODO: 
                        // remove shared border cols/rows
                        // add 1 at end

                        // calculate width in columns
                        window.cols = x_segm_cols [x];
                        int xi;
                        for (xi = x+1; xi < x_ratio; xi++) {
                            if (ch == layout_matrix [y][xi]) {
                                window.cols [i] += x_segm_cols [xi];
                            } else {
                                break;
                            }
                        }

                        // calculate height in rows
                        window.rows = y_segm_rows [y];
                        int yi;
                        for (yi = y+1; yi < y_ratio; yi++) {
                            if (ch == layout_matrix [yi][x]) {
                                window.rows += y_segm_rows [yi];
                            } else {
                                break;
                            }
                        }

                        // TODO: 
                        // mark used segments (un)used segment matrix


                        used_segm_matrix [x][y] = 1;
                    }
                }
            }
            window = NULL;

            // 


            move(3+i, 4);
            printw("%d (* %d = %d) / %d (* %d = %d)   %s", 
                    x_ratio, x_segm_len,  x_segm_len * x_ratio,
                    y_ratio, y_segm_len,  y_segm_len * y_ratio, 
                    data.categ_labels[i] );
        }
    }
}

/*
*/
static struct window* 
create_window_struct ()
{
    struct window *window = (struct window*) malloc (sizeof (struct window));

    if (window)
        return window;

    else {
        pfem ("Unable to create window struct");
        exit (EXIT_FAILURE);
    }
}
