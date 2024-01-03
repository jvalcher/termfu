
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

        category [0] = '\0';
        label [0] = '\0';
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
    Allocate memory for new windows_t struct
*/
static plugin_t *allocate_plugin (void)
{
    plugin_t *plugin = (plugin_t *) calloc (1, sizeof (plugin_t));
    if (plugin) {
        return plugin;
    } else {
        endwin ();
        pfem ("Unable to allocate memory for plugin_t struct");
        exit (EXIT_FAILURE);
    }
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
    char section = 0;
    int num_chars = 0;
    bool is_key = false;
    char keys [MAX_KEY_STR_LEN] = {0};
    int i = 0;

    // increment number of layouts
    layouts->num += 1;

    // get layout index
    int li = layouts->num - 1;

    // add layout label
    strncpy (layouts->labels [li], label, strlen (label));

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
                        layouts->plugins[li] = curr_plugin;
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
                layouts->hdr_key_rows[li] = 0;

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
                        layouts->hdr_key_rows[li] += 1;
                    keys [i++] = ch;
                    num_chars += 1;
                } 
            }
            keys [i] = '\0';

                // add string to layouts
            if (section == 'h') {
                strncpy ((char *)layouts->hdr_key_strs[li], keys, num_chars + 1);
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
    //  * * *     layouts->row_ratios [li] == 2
    //  * * * --> layouts->col_ratios [li] == 3
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

        // add ratio to layouts
    layouts->row_ratios [li] = y_ratio;
    layouts->col_ratios [li] = x_ratio;


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

        // add matrix to layouts
    layouts->win_matrices [li] = (char *) layout_matrix;
}



/*
    Print layout, plugin information for debugging
*/
#ifdef LAYOUT

/*
    Convert {0..52} values to {0,a..z,A..Z}
*/
int index_to_key (int n)
{
    if (n >= 1 && n <= 26)
        return n + 'a' - 1;
    else if (n >= 27 && n <= 52)
        return n + 'A' - 27;
    else
        return -1;
}

/*
    Print:
        - Data for first <n> layouts
        - Current plugin key bindings
    ---------
    Called in render_layout.c -> render_layout()

    Run with:

        $ make layouts
*/
void print_layouts (int n, 
                    layouts_t *layouts)
{
    // print layouts data
    int i, k, l;
    int col;
    static int row = 1;

    for (i = 0; i < n; i++) {

        // labels, ratios
        mv_print_title (BLUE_BLACK, stdscr, row, 1, layouts->labels [i]);

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
        mvprintw (++row, 0, "%s", (char*)layouts->hdr_key_strs[i]);

        // window ratios
        row += layouts->hdr_key_rows[i];
        mv_print_title (GREEN_BLACK, stdscr, ++row, 1, "Window segment ratios");
        mvprintw (++row, 1, "row: %d",
                layouts->row_ratios [i]);
        mvprintw (++row, 1, "col: %d",
                layouts->col_ratios [i]);

        // window matrix
        char **matrix = (char **) layouts->win_matrices [i];
        row += 2;
            // symbols
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Window segment matrix");
        row += 1;
        for (k = 0; k < layouts->row_ratios [i]; k++) {
            col = 1;
            for (l = 0; l < layouts->col_ratios [i]; l++) {
                mvprintw (row, col, "%c", matrix [k][l]);
                col += 1;
            }
            row += 1;
        }

        // rows, cols
        row += 1;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Segment rows x cols");
        row += 1;
        for (k = 0; k < layouts->row_ratios [i]; k++) {
            col = 1;
            for (l = 0; l < layouts->col_ratios [i]; l++) {
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
        int save_row = row;
        int column_rows = 10;
        int total_rows = 0;
        mv_print_title (GREEN_BLACK, stdscr, row, 1, "Plugins");
        plugin_t* curr_plugin = layouts->plugins[i];
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
        for (k = 0; k < 2; k++) {
            col = 1;
            for (l = 0; l < 26; l++) {

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
    }

    getch ();
}
#endif
