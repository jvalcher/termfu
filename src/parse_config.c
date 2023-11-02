
/*
    Parse configuration file
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <ncurses.h>

#include "parse_config.h"
#include "data.h"
#include "utilities.h"

static FILE* open_config_file (void);
static void get_category_and_label (FILE* file, char *category, char *label);
static void create_layout (FILE* file, layouts_t *layouts, char *label);
static plugins_t *allocate_plugin (void);


/*
    Parse CONFIG_FILE data into passed layouts_t object

        [<category>:<label>]
        <value>
*/
void parse_config (layouts_t *layouts)
{

    // open configuration file
    FILE *file = open_config_file ();

    // parse CONFIG_FILE data
    int  i;
    char ch;
    char category [MAX_CONFIG_CATEG_LEN];
    char label    [MAX_CONFIG_LABEL_LEN];

    while ((ch = fgetc (file)) != EOF) {

        // get category and label of new setting
        if (ch == '[') 
            get_category_and_label (file, category, label);

        // create layout
        if (strcmp (category, "layout") == 0)
            create_layout (file, layouts, label);

        // zero out category, label, value
        i = 0;
        while (category [i] != '\0') category [i++] = '\0';
        i = 0;
        while (label [i] != '\0') label [i++] = '\0';
    }

    // close configuration file
    fclose (file);

    // set screen height, width to 0 to trigger layout
    // creation in render_screen.c
    layouts->scr_height = 0;
    layouts->scr_width  = 0;
}


/*
    Open config file
    ---------
    Checks in this order:

        PWD/.termide
        HOME/.termide
*/
static FILE* open_config_file (void)
{
    char cwd[100];
    char cwd_path[130];
    char home_path[130];
    FILE *file;

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
        pfemo ("%s/.termide\n", cwd_path);
        pfemo ("%s/.termide\n", home_path);

        exit (EXIT_FAILURE);
    }

    return file;
}



/*
    Get category and label
*/
static void get_category_and_label (FILE *file,
                                    char *category,
                                    char *label)
{
    int i;
    int ch = 0;

    do {

        // category
        i = 0;
        while ((ch = fgetc (file)) != ':'  &&
                    i < MAX_CONFIG_CATEG_LEN - 1) {
            if (ch != ' ')
                category [i++] = ch;
        }
        category [i] = '\0'; 

        // label
        i = 0;
        while ((ch = fgetc (file)) != ']' &&
                    i < MAX_CONFIG_LABEL_LEN - 1) {
            label [i++] = ch;
        }
        label [i] = '\0';

    } while (ch != ']');
}



/*
    Create new layout
*/
static void create_layout (FILE* file,
                           layouts_t *layouts,
                           char *label)
{
    int ch, next_ch;
    char win_keys [MAX_KEY_STR_LEN] = {0};
    int i = 0;

    // increment number of layouts
    layouts->num += 1;

    // get layout index
    int li = layouts->num - 1;

    // add layout label
    strncpy (layouts->labels [li], label, strlen (label));

    //
    // parse configuration
    //
    char section = 0;
    while ((ch = fgetc(file)) != '['  &&  ch != EOF) {

        // set section (p, h, w)
        if (ch == '>') {
            next_ch = fgetc (file);
            switch (next_ch) {
                case 'p':
                    section = 'p';
                    break;
                case 'h':
                    section = 'h';
                    break;
                case 'w':
                    section = 'w';
                    break;
            }
        }

        // add plugins, keys, titles
        int num_plugins = 0;

        // parse plugins
        if (section == 'p') {
            
            // plugin index
            int pi = 0;

            // allocate plugins_t object
            layouts->plugins [li] = allocate_plugin();

            // increment number plugins
            layouts->plugins[li]->num = num_plugins++;

            // parse header
            while ((ch = fgetc (file)) != '[' && 
                    ch != '>' && 
                    ch != EOF &&
                    pi < NUM_PLUGINS) {

                // if letter
                if (isalpha (ch)) {

                    // get code
                    for (int j = 0; j < 3; j++) {
                        layouts->plugins[li]->codes[pi][j] = ch;
                        ch = fgetc (file);
                    }
                    layouts->plugins[li]->codes[pi][3] = '\0';

                    // get key
                    ungetc (ch, file);
                    while ((ch = fgetc (file)) != ':') {;}
                    while ((ch = fgetc (file)) != ':') {
                        if (isalpha (ch)) {
                            layouts->plugins[li]->keys[pi][0] = ch;
                        }
                    }

                    // get title
                    i = 0;
                    while ((ch = fgetc (file)) != '\n' && i < (MAX_CONFIG_LABEL_LEN - 1)) {
                        layouts->plugins[li]->titles[pi][i++] = ch;
                    }
                    layouts->plugins[li]->titles[pi][i] = '\0';

                    // increment plugin index
                    pi += 1;
                    layouts->plugins[li]->num += 1;
                }
            }

            // unget '>' or '['
            ungetc (ch, file);
            section = 0;
        }

        // create header, window key strings
        //
        //   ssb
        //   ssw
        //   ccr     -->  "ssb\nssw\nccr\n"
        //
        i = 0;
        bool is_key = false;
        char keys [MAX_KEY_STR_LEN] = {0};
            //
        if (section == 'w' || section == 'h') {

            if (section == 'h')
                layouts->hdr_key_rows[li] = 0;

            while ((ch = fgetc (file)) != '>' && ch != '[' && ch != EOF) {

                // add key
                if (isalpha(ch)) {
                    keys [i++] = ch;
                    is_key = true;
                }

                // add newline
                else if (ch == '\n' && 
                         is_key == true && 
                         keys [i-1] != '\n') {

                    keys [i++] = '\n';

                    // increment number of header rows
                    if (section == 'h')
                        layouts->hdr_key_rows[li] += 1;
                } 
            }
            keys [i] = '\0';

            // store string
            if (section == 'h') {
                strcpy ((char *)layouts->hdr_key_strs[li], keys);
            } else {
                strcpy (win_keys, keys);
            }

            // unget '>' or '['
            ungetc (ch, file);
            section = 0;
        }
    }

    // unget '>' or '['
    ungetc (ch, file);

    // calculate window segment ratio
    //
    //  * * *     layouts->row_ratios [li] == 2
    //  * * * --> layouts->col_ratios [li] == 3
    //
    // "segment" refers to the space needed for each window symbol character
    //      
    //   i.e. ssb
    //        ssw
    //        ccr --> (s) window has 2x2 segment area
    //                i.e. window is 2/3 of screen width and height
    //
    int y_ratio   = 0;
    int x_ratio   = 0;
    bool  x_count = true;
    for (i = 0; i < strlen (win_keys); i++) {
        if (win_keys [i] == '\n') {
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

        // add keys
    int row = 0;
    int col = 0;
    for (i = 0; i < strlen (win_keys); i++) {
        layout_matrix [row][col] = win_keys [i];
        if (col < x_ratio - 1) {
            col += 1;
        } else {
            col  = 0;
            row += 1;
            i   += 1;     // skip '\n'
        }
    }

        // add to layouts
    layouts->win_matrices [li] = (char *) layout_matrix;
}



/*
    Allocate memory for new windows_t struct
*/
static plugins_t *allocate_plugin (void)
{
    plugins_t *plugins = (plugins_t *) calloc (1, sizeof (plugins_t));
    if (plugins) {
        return plugins;
    } else {
        endwin ();
        pfem ("Unable to allocate memory for plugins_t struct");
        exit (EXIT_FAILURE);
    }
}




#ifdef LAYOUT
/*
    Print data for first <n> layouts, pause  (debugging)
    ---------
    Set n with: 

        src/data.h : PRINT_LAYOUTS

    Call with:

        $ make layouts
*/
void print_layouts (int n, 
                    layouts_t *layouts)
{
    // print layouts data
    int col;
    static int row = 1;

    for (int i = 0; i < n; i++) {

        // labels, ratios
        mv_print_title (BLUE_BLACK, stdscr, ++row, 1, "%s", layouts->labels [i]);

        // screen dimensions
        row += 1;
        mv_print_title (GREEN_BLACK, stdscr, ++row, 1, "Screen size:");
        mvprintw (++row, 1, "cols: %d",
                getmaxy (stdscr));
        mvprintw (++row, 1, "rows: %d",
                getmaxx (stdscr));

        // header keys
        row += 2;
        mv_print_title (GREEN_BLACK, stdscr, row++, 1, "Header plugin keys:");
        mvprintw (++row, 0, "%s", (char*)layouts->hdr_key_strs[i]);

        // window ratios
        row += layouts->hdr_key_rows[i] + 1;
        mv_print_title (GREEN_BLACK, stdscr, ++row, 1, "Window segment ratios:");
        mvprintw (++row, 1, "row: %d",
                layouts->row_ratios [i]);
        mvprintw (++row, 1, "col: %d",
                layouts->col_ratios [i]);

        // window matrix
        char **matrix = (char **) layouts->win_matrices [i];
        row += 2;
            // symbols
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Window segment matrix:");
        row += 2;
        for (int k = 0; k < layouts->row_ratios [i]; k++) {
            col = 0;
            for (int l = 0; l < layouts->col_ratios [i]; l++) {
                mvprintw (row, col, "%c", matrix [k][l]);
                col += 1;
            }
            row += 1;
        }

        // rows, cols
        row += 1;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Segment rows x cols:");
        row += 2;
        for (int k = 0; k < layouts->row_ratios [i]; k++) {
            col = 1;
            for (int l = 0; l < layouts->col_ratios [i]; l++) {
                mvprintw (row, col, "(%c) %dx%d", 
                        matrix [k][l], 
                        layouts->windows[i]->rows,
                        layouts->windows[i]->cols);
                col += 12;
            }
            row += 1;
        }

        // plugins
        col = 1;
        row += 1;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Plugins:");
        row += 1;
        for (int k = 0; k < layouts->plugins[i]->num; k++) {
            mvprintw (++row, col, "%s : %c : %s",
                    (char *) layouts->plugins[i]->codes[k],
                    layouts->plugins[i]->keys[k][0],
                    (char *) layouts->plugins[i]->titles[k]);
        }

        row += 2;
    }

    getch ();
}
#else
void print_layouts (int n, layouts_t *layouts) {}       // dummy function
#endif
