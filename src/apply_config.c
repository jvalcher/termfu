
/*
    Parse configuration file
    ----------------

    Contents:

        parse_config ()             #main       
        get_config_file_data ()     #file       
        parse_layout ()             #layout     

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <ncurses.h>

#include "parse_config.h"
#include "utilities.h"

#define CONFIG_FILE             ".gdb-tuiffic"
#define MAX_CONFIG_ENTRIES      20
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    MAX_CONFIG_CATEG_LEN
#define MAX_CONFIG_VALUE_LEN    100

struct config_file_data {
    int     num_configs;
    char    categories      [MAX_CONFIG_ENTRIES][MAX_CONFIG_CATEG_LEN];
    char    categ_labels    [MAX_CONFIG_ENTRIES][MAX_CONFIG_LABEL_LEN];
    char    categ_values    [MAX_CONFIG_ENTRIES][MAX_CONFIG_VALUE_LEN];
};

static void get_config_file_data (struct config_file_data*);
static void parse_layout (int, int, struct config_file_data*, struct layouts*, int, int);


/*
    Parse config_file_data struct (get_config.h)
    Pass resulting data into configuration structs (apply_config_*.h)

    #main
*/
void parse_config (struct layouts *layouts)
{
    // calculate current terminal pane dimensions
    int scr_cols = getmaxx (stdscr);
    int scr_rows = getmaxy (stdscr) - 2;   // minus program title bar

    // get config file data struct
    struct config_file_data data = {0};
    get_config_file_data (&data);

    // create configuration structs for
        // layouts
    layouts = (struct layouts*) malloc (sizeof (struct layouts));
    if (layouts == NULL) {
        endwin ();
        pfem ("layouts struct is null");
        exit (EXIT_FAILURE);
    }
    layouts->num_layouts = 0;

    // loop through configurations
    for (int i = 0; i < data.num_configs; i++) {

        // if layout
        if (strcmp (data.categories[i], "layout") == 0) {
            layouts->num_layouts += 1;
            apply_config_layout (di, li, &data, layouts, scr_rows, scr_cols);
        }                        
    }
}


/*
    Add data from CONFIG_FILE to config_file_data struct
    ------

    Searches:

        PWD/CONFIG_FILE
        HOME/CONFIG_FILE

    #file
*/
static void get_config_file_data (struct config_file_data *data)
{
    FILE *file = NULL;
    char cwd[100];
    char cwd_path[130];
    char home_path[130];

    // check current working directory
    if (getcwd (cwd, sizeof (cwd)) != NULL) {
        snprintf (cwd_path, sizeof (cwd_path), "%s/%s", cwd, CONFIG_FILE);
        file = fopen (cwd_path, "r");
    }

    // check in user's home directory
    if (file == NULL) {
        char *home = getenv ("HOME");
        snprintf (home_path, sizeof (home_path), "%s/%s", home, CONFIG_FILE);
        file = fopen (home_path, "r");
    }

    // if no config file found, exit
    if (file == NULL) {

        endwin ();

        pfem ("Unable to find config file in home or current working directory");
        pfemo (cwd_path);
        pfemo (home_path);

        exit (EXIT_FAILURE);
    }

    // Add data to config_file_data
    char ch;
    bool config_found = false;
    data->num_configs = 0;

        // create config_file_data row indexes
    int i_cat = 0;
    int i_lab = 0;
    int i_val = 0;

        // read file
    while ((ch = fgetc(file)) != EOF) {

        int i;

        // get [<category>:<label>]
        if (ch == '[') {
            config_found = true;

            // get category
            i = 0;
            while ( (ch = fgetc(file)) != ':'  &&
                    i < MAX_CONFIG_CATEG_LEN - 1  &&
                    i_cat < MAX_CONFIG_ENTRIES ) {

                data->categories[i_cat][i++] = ch;
            }
            data->categories[i_cat++][i] = '\0'; 

            // get label
            i = 0;
            while ( (ch = fgetc(file)) != ']' &&
                    i < MAX_CONFIG_CATEG_LEN - 1  &&
                    i_cat < MAX_CONFIG_ENTRIES ) {

                data->categ_labels[i_lab][i++] = ch;
            }
            data->categ_labels[i_lab++][i] = '\0';
            data->num_configs += 1;
        }     

        // get config value
        if (config_found == true) {

            // add layout
            if (strcmp(data->categories[i_cat-1], "layout") == 0) {

                // create data->categ_values[i_val] string
                i = 0;
                bool is_val = false;
                while ( (ch = fgetc(file)) != '['  &&  ch != EOF ) {

                    // add letter
                    if (isalpha(ch)) {
                        data->categ_values[i_val][i++] = ch;
                        is_val = true;
                    }

                    // add 'n' for newline
                    else if (   ch == '\n' && 
                                is_val == true && 
                                data->categ_values[i_val][i-1] != 'n' // no duplicates
                            )   {
                        data->categ_values[i_val][i++] = 'n';
                    } 
                }
                data->categ_values[i_val][i] = '\0';
                i_val += 1;
                ungetc(ch, file);       // unget '['
                config_found = false;
            }
        }
    }

    fclose (file);
}





/*

    Parse config file layout
    --------------
    Add window size, position data from config_file_data 
    struct (get_config.h) to layouts struct

    Layout parsing example:

        CONFIG_FILE -->

            [layout:my_layout_1]
            cwbr
            aacc
            aacc

        get_config_file_data (data) -->

            num_configs       ==  1
            categories   [i]  ==  "layout"
            categ_labels [i]  ==  "my_layout_1"
            categ_values [i]  ==  "cwbrnaaccnaaccn"

    #layout
*/
static void parse_layout (int di,
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
