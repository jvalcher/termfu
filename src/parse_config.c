
/*
    Parse configuration file
    ----------------

    Contents:

        Tag         Description
        ---         -----------
        #open       Open CONFIG_FILE
        #parse      Parse [<layout>:<label>]
        #keys       Parse key bindings
        #layout     Parse layout
        #print      Print layout data
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

/*
    Action codes
    -----
    Global identifiers, linked below in parse_config() with shortcut keys (keys)
    set in the configuration file, are used to identify a specific "action"
    implemented in actions.c

        wa:w:(w)atch
  
        <code> : <key> : <window title>
*/
char *action_codes [NUM_ACTIONS] = {
    "as",       // display assembly data
    "br",       // display breakpoints
    "wa",       // display watched variables
    "lv",       // display local variables
    "sc",       // display source file
    "rg",       // display registers
    "ou"        // display action output
};

char *window_names [NUM_ACTIONS] = {
    "Assembly",
    "Breakpoints",
    "Watches",
    "Local vars",
    "Source",
    "Registers",
    "Output"
};

char action_keys [NUM_ACTIONS] = {'a', 'b', 'w', 'l', 's', 'r', 'o'};


/*
    Parse CONFIG_FILE data into passed configuration struct pointers
*/
void parse_config (layouts_t *layouts)
{

// #open

    // Open CONFIG_FILE
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

        pfem  ("Unable to find config file:\n");
        pfemo ("%s\n", cwd_path);
        pfemo ("%s\n", home_path);

        exit (EXIT_FAILURE);
    }

// #parse

    // parse CONFIG_FILE data
    //
    //  [<category>:<label>]
    //  <value>
    //
    int  i;
    char ch, next_ch;
    bool is_config = false;
    char category [MAX_CONFIG_CATEG_LEN];
    char label    [MAX_CONFIG_LABEL_LEN];
    char value    [MAX_CONFIG_VALUE_LEN];

        // loop through file characters
    while ((ch = fgetc (file)) != EOF) {

        // get next character to check for double brackets '[['
        next_ch = fgetc (file);
        ungetc (next_ch, file);

        // get [[<category>:<label>]] strings
        if (ch == '[' && next_ch == '[') {

            // get second "["
            ch = fgetc (file);

            // category
            i = 0;
            while ((ch = fgetc(file)) != ':'  &&
                     i < MAX_CONFIG_CATEG_LEN - 1) {
                category [i++] = ch;
            }
            category [i] = '\0'; 

            // label
            i = 0;
            while ((ch = fgetc(file)) != ']' &&
                     i < MAX_CONFIG_LABEL_LEN - 1) {
                label [i++] = ch;
            }
            label [i] = '\0';

            // set configuration toggle
            is_config = true;

            // get second "]"
            ch = fgetc (file);
        }     

// #keys

// #layout

        // if layout
        if (is_config == true && strcmp (category, "layout") == 0) {

            // increment number of layouts
            layouts->num += 1;

            // get layout index
            int li = layouts->num - 1;

            // add label string to layouts
            strncpy (layouts->labels [li], label, MAX_LABEL_LEN);

            // create window symbol string
            //
            //   ssb
            //   ssw
            //   ccr  -->  value == "ssbnsswnccrn"
            //
            i = 0;
            bool is_val = false;
            while ((ch = fgetc(file)) != '['  &&  ch != EOF) {

                // add letter
                if (isalpha(ch)) {
                    value [i++] = ch;
                    is_val = true;
                }

                // add 'n' for newline
                else if (   ch == '\n' && 
                            is_val == true && 
                            value [i-1] != 'n' // no duplicates
                        )   {
                    value [i++] = 'n';
                } 
            }
            value [i] = '\0';

            // unget '[' for next loop
            ungetc (ch, file);

            // calculate window segment ratio
            //
            //  * * *     layouts->row_ratios [li] == 2
            //  * * * --> layouts->col_ratios [li] == 3
            //
            // *A segment refers to a single window symbol character
            //      
            //   i.e. ssb
            //        ssw
            //        ccr --> The (s)ource window has four segments
            //
            int y_ratio = 0;
            int x_ratio = 0;
            bool  x_count           = true;
            for (i = 0; i < strlen (value); i++) {
                if (value [i] == 'n') {
                    y_ratio += 1;
                    x_count = false;
                }
                if (x_count) {
                    x_ratio += 1;
                }
            }

                // add to layouts
            layouts->row_ratios [li] = y_ratio;
            layouts->col_ratios [li] = x_ratio;

            // create layout matrix
            //
            //   value -->  {{s,s,b},
            //               {s,s,w},
            //               {c,c,r}}
            //
                // allocate matrix
            char **layout_matrix = (char **) malloc (y_ratio * sizeof (char*));
            for (i = 0; i < y_ratio; i++) {
                layout_matrix [i] = (char *) malloc (x_ratio * sizeof (char));
            }
            if (layout_matrix == NULL) {
                endwin ();
                pfem ("Unable to create layout_matrix for %s\n", 
                            layouts->labels [i]);
                exit (EXIT_FAILURE);
            }

                // add symbols
            int row = 0;
            int col = 0;
            for (i = 0; i < strlen (value); i++) {
                layout_matrix [row][col] = value [i];
                if (col < x_ratio - 1) {
                    col += 1;
                } else {
                    col  = 0;
                    row += 1;
                    i   += 1;     // skip 'n'
                }
            }

                // add to layouts
            layouts->matrices [li] = (char *) layout_matrix;

            // unset configuration toggle
            is_config = false;
        }

        // zero category, label, value
        i = 0;
        while (category [i] != '\0') category [i++] = '\0';
        i = 0;
        while (label [i] != '\0') label [i++] = '\0';
        i = 0;
        while (value [i] != '\0') value [i++] = '\0';
    }

    // close configuration file
    fclose (file);

    // set screen height, width to 0 to trigger layout
    // creation in render_screen.c
    layouts->scr_height = 0;
    layouts->scr_width  = 0;
}


/*
    Print data for first <n> layouts
*/
// #print
void print_layouts (
        int n, 
        layouts_t *layouts)
{
    // print layouts data
    int col;
    int row = 4;
    windows_t *windows = NULL;
    for (int i = 0; i < n; i++) {

        // labels, ratios
        move (row++, 2);
        printw ("--%s--   ratio: %dx%d   screen: %dx%d\n\n",
                layouts->labels [i],
                layouts->row_ratios [i],
                layouts->col_ratios [i],
                getmaxy (stdscr),
                getmaxx (stdscr));

        // matrix data
        char **matrix = (char **) layouts->matrices [i];
            // symbols
        for (int k = 0; k < layouts->row_ratios [i]; k++) {
            col = 3;
            for (int l = 0; l < layouts->col_ratios [i]; l++) {
                move (row, col);
                printw ("%c", matrix [k][l]);
                col += 1;
            }
            row += 1;
        }

        col = 2;
        move (row++, col);
        printw ("---------");

            // rows,cols
        for (int k = 0; k < layouts->row_ratios [i]; k++) {
            col = 3;
            for (int l = 0; l < layouts->col_ratios [i]; l++) {
                move (row, col);
                printw ("(%c) %dx%d", 
                        matrix [k][l], 
                        layouts->windows[i]->rows,
                        layouts->windows[i]->cols);
                col += 12;
            }
            row += 1;
        }

        col = 3;
        move (row++, col);
        printw ("---------");

        // window values
        windows = layouts->windows [i];
        do {
            move (row, col);
            printw ("%c (%d,%d) %dx%d",
                    windows->key,
                    windows->y,
                    windows->x,
                    windows->rows,
                    windows->cols);
            row += 1;
            windows = windows->next;
        } while (windows->next != NULL);

        row += 1;
    }
}

