
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
static layout_t* create_layout (FILE* file, char *label);


/*
    Parse CONFIG_FILE data into passed layout_t object

        [<category>:<label>]
        <value>
*/
layout_t* parse_config (void)
{
    // open configuration file
    FILE *file = open_config_file ();

    // parse CONFIG_FILE data
    char ch;
    char category [MAX_CONFIG_CATEG_LEN];
    char label    [MAX_CONFIG_LABEL_LEN];
    bool first_layout = true;
    layout_t *head_layout = NULL;
    layout_t *curr_layout = NULL;

    while ((ch = fgetc (file)) != EOF) {

        // get category and label of new setting
        if (ch == '[') 
            get_category_and_label (file, category, label);

        // create layout
        if (strcmp (category, "layout") == 0) {
            curr_layout = create_layout (file, label);
            if (first_layout) {
                head_layout = curr_layout;
                first_layout = false;
            }
            curr_layout = curr_layout->next;
        }

        category [0] = '\0';
        label [0] = '\0';
    }

    // close configuration file
    fclose (file);

    return head_layout;
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
        pfem   ("Unable to find config file:\n");
        pfemo  ("%s/.termide\n", cwd_path);
        pfemoe ("%s/.termide\n", home_path);
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
    Allocate memory for layout_t struct
*/
static layout_t* allocate_layout (void)
{
    void *config_ptr = (layout_t*) malloc (sizeof (layout_t));

    if (config_ptr == NULL) {
        pfeme ("layout_t allocation failed\n");
    } else {
        return config_ptr;
    }
}



/*
    Allocate memory for new windows_t struct
*/
static plugin_t *allocate_plugin (void)
{
    plugin_t *plugin = (plugin_t *) calloc (1, sizeof (plugin_t));
    if (plugin) {
        return plugin;
    } else {
        pfeme ("Unable to allocate memory for plugin_t struct");
    }
}



/*
    Create new layout
*/
static layout_t* create_layout (FILE* file,
                           char *label)
{
    int ch, next_ch;
    char win_keys [MAX_KEY_STR_LEN] = {0};
    char section = 0;
    int num_chars = 0;
    bool is_key = false;
    char keys [MAX_KEY_STR_LEN] = {0};
    int i = 0;
    layout_t* layout = NULL;

    // allocate new layout_t struct
    layout = allocate_layout ();
    layout->next = NULL;

    // add layout label
    strncpy (layout->label, label, strlen (label));

    //
    // Parse configuration
    //
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

        // parse plugins
        //
        //      >p
        //      Bld: b : (b)uild
        //      Src: f : source (f)ile
        //
        if (section == 'p') {
            
            plugin_t* curr_plugin = NULL;
            bool curr_plugin_exists = false;

            // parse header
            while ((ch = fgetc (file)) != '[' && 
                    ch != '>' && 
                    ch != EOF) {

                // if letter
                if (isalpha (ch)) {

                    // allocate plugin
                    if (curr_plugin_exists) {
                        curr_plugin->next = allocate_plugin ();
                        curr_plugin = curr_plugin->next;
                    } else {
                        curr_plugin = allocate_plugin();
                        layout->plugins = curr_plugin;
                        curr_plugin_exists = true;
                    }
                    curr_plugin->next = NULL;

                    // get code
                    for (int j = 0; j < 3; j++) {
                        curr_plugin->code[j] = ch;
                        ch = fgetc (file);
                    }
                    curr_plugin->code[3] = '\0';

                    // get key
                    ungetc (ch, file);
                    while ((ch = fgetc (file)) != ':') {;}
                    while ((ch = fgetc (file)) != ':') {
                        if (isalpha (ch)) {
                            curr_plugin->key = ch;
                        }
                    }

                    // get title
                    i = 0;
                    while ((ch = fgetc (file)) != '\n' && i < (MAX_CONFIG_LABEL_LEN - 1)) {
                        curr_plugin->title[i++] = ch;
                    }
                    curr_plugin->title[i] = '\0';
                }
            }

            // unget '>' or '['
            ungetc (ch, file);
        }
        // end plugin parse


        // parse header or window section
        if (section == 'w' || section == 'h') {

            i = 0;
            num_chars = 0;

            // zero out number of header rows
            if (section == 'h')
                layout->num_hdr_key_rows = 0;

            // create header, window key string
            //
            //   >w
            //   ssb
            //   ssw
            //   ccr     -->  "ssb\nssw\nccr\n"
            //
            while ((ch = fgetc (file)) != '>' && 
                    ch != '[' && 
                    ch != EOF &&
                    num_chars <= MAX_KEY_STR_LEN) {

                // add key
                if (isalpha(ch)) {
                    keys [i++] = ch;
                    num_chars += 1;
                    is_key = true;
                }

                // add newline
                else if (ch == '\n' && 
                         is_key == true && 
                         keys [i-1] != '\n' &&
                         i != 0) {

                    if (section == 'h')
                        layout->num_hdr_key_rows += 1;
                    keys [i++] = ch;
                    num_chars += 1;
                } 
            }
            keys [i] = '\0';

                // add string to layout
            if (section == 'h') {
                strncpy ((char *)layout->hdr_key_str, keys, num_chars + 1);
            } else {
                strncpy (win_keys, keys, num_chars + 1);
            }

            // unget '>' or '['
            ungetc (ch, file);

            num_chars = 0;
        }
        // end header, window section

        section = 0;
    }
    // end config section parse

    // unget '>' or '['
    ungetc (ch, file);

    // calculate terminal screen or pane window segment ratio
    //
    //  * * *     layout->row_ratio == 2
    //  * * * --> layout->col_ratio == 3
    //
    //  A "segment" refers to the space needed for each window symbol character
    //  in terms of the current terminal's dimensions.
    //      
    //   e.g. ssbb
    //        ssww
    //        ccrr --> The 's' window will take up half of the screen's
    //                 columns (width) and two thirds of the rows (height).
    //
    int y_ratio   = 0;
    int x_ratio   = 0;
    bool  x_count = true;
    for (i = 0; i < strlen (win_keys); i++) {
        if (win_keys [i] == '\n') {
            y_ratio += 1;
            x_count = false;
        }
        if (x_count == true && isalpha(win_keys[i])) {
            x_ratio += 1;
        }
    }

        // add ratio to layout
    layout->row_ratio = y_ratio;
    layout->col_ratio = x_ratio;


    // create window matrix
    //
    //     {{s,s,b},
    //      {s,s,w},
    //      {c,c,r}}
    //
        // allocate matrix
    char **layout_matrix = (char **) malloc (y_ratio * sizeof (char*));
    for (i = 0; i < y_ratio; i++) {
        layout_matrix [i] = (char *) malloc (x_ratio * sizeof (char));
    }
    if (layout_matrix == NULL) {
        pfeme ("Unable to create layout_matrix for %s\n", 
                    layout->label);
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

        // add matrix to layout
    layout->win_matrix = (char *) layout_matrix;

    // return next layout
    return layout;
}



#ifdef LAYOUT
/*
    Print layout, plugin information for debugging
    ---------
    - Data for first <n> layout
    - Current layout's plugin key bindings
    - Called in render_layout.c -> render_layout()

    Run with:

        $ make layouts
*/
void print_layouts (int n, 
                    layout_t *layout)
{
    // print layout data
    int k, l;
    int col;
    static int row = 1;

    do {

        // labels, ratios
        mv_print_title (BLUE_BLACK, stdscr, row, 1, layout->label);

        // screen dimensions
        row += 1;
        mv_print_title (GREEN_BLACK, stdscr, ++row, 1, "Screen size");
        mvprintw (++row, 1, "cols: %d",
                getmaxy (stdscr));
        mvprintw (++row, 1, "rows: %d",
                getmaxx (stdscr));

        // header keys
        row += 2;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Header plugin keys");
        mvprintw (++row, 0, "%s", (char*)layout->hdr_key_str);

        // window ratios
        row += layout->num_hdr_key_rows;
        mv_print_title (GREEN_BLACK, stdscr, ++row, 1, "Window segment ratios");
        mvprintw (++row, 1, "row: %d",
                layout->row_ratio);
        mvprintw (++row, 1, "col: %d",
                layout->col_ratio);

        // window matrix
        char **matrix = (char **) layout->win_matrix;
        row += 2;
            // symbols
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Window segment matrix");
        row += 1;
        for (k = 0; k < layout->row_ratio; k++) {
            col = 1;
            for (l = 0; l < layout->col_ratio; l++) {
                mvprintw (row, col, "%c", matrix [k][l]);
                col += 1;
            }
            row += 1;
        }

        // rows, cols
        row += 1;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Segment rows x cols");
        row += 1;
        for (k = 0; k < layout->row_ratio; k++) {
            col = 1;
            for (l = 0; l < layout->col_ratio; l++) {
                mvprintw (row, col, "(%c) %dx%d", 
                        matrix [k][l], 
                        layout->windows->rows,
                        layout->windows->cols);
                col += 12;
            }
            row += 1;
        }

        // plugins
        col = 1;
        row += 1;
        int save_row = row;
        int column_rows = 10;
        int total_rows = 0;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Plugins");
        plugin_t* curr_plugin = layout->plugins;
        do {
            total_rows += 1;
            if (total_rows > column_rows) {
                col += 30;
                row -= 10;
                total_rows = 0;
            }

            mvprintw (++row, col, "%s : %c : %s",
                    (char *) curr_plugin->code,
                    curr_plugin->key,
                    (char *) curr_plugin->title);

            curr_plugin = curr_plugin->next;

        } while (curr_plugin != NULL);

        // keyboard shortcut, function index
        col = 1;
        row = save_row + column_rows + 2;
        int index = 1;
        int ch;
        mv_print_title (GREEN_BLACK, stdscr, row++, 1, "Key binding : function index");
            //
        for (k = 0; k < 4; k++) {
            col = 1;
            for (l = 0; l < 13; l++) {

                // convert index to key shortcut
                if (index >= 1 && index <= 26)
                    ch = index + 'a' - 1;
                else if (index >= 27 && index <= 52)
                    ch = index + 'A' - 27;

                mvprintw (row, col, "%c:%d ", ch, key_function_index[index++]);
                col += 5;
            }
            row += 1;
        }

        layout = layout->next;

        getch ();

    } while (layout != NULL);
}
#endif
