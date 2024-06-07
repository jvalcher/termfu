
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

static FILE      *open_config_file       (void);
static void       get_category_and_label (FILE*, char*, char*);
static void       create_plugins         (FILE*, state_t*);
static layout_t  *create_layout          (FILE*, state_t*, char*);
static layout_t  *allocate_layout        (void);
static plugin_t  *allocate_plugin        (void);


/*
    Parse CONFIG_FILE data into passed layout_t object

        [<category>:<label>]
        <values>
*/
void parse_config (state_t *state)
{
    FILE *file = open_config_file ();

    char ch;
    char category [MAX_CONFIG_CATEG_LEN];
    char label    [MAX_CONFIG_LABEL_LEN];
    bool first_layout = true;
    layout_t *curr_layout = NULL;

    while ((ch = fgetc (file)) != EOF) {

        // get category and label of new setting
        if (ch == '[') 
            get_category_and_label (file, category, label);

        // create all plugins
        if (strcmp (category, "plugins") == 0) {
            create_plugins (file, state);
        }

        // create layout
        if (strcmp (category, "layout") == 0) {

            curr_layout = create_layout (file, state, label);

            if (first_layout) {
                state->layouts = curr_layout;
                first_layout = false;
            }
            curr_layout = curr_layout->next;
        }

        category [0] = '\0';
        label [0] = '\0';
    }

    // close configuration file
    fclose (file);
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
    -------
        [ <categ> : <label> ]
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

        // label (if present)
        i = 0;
        while ((ch = fgetc (file)) != ']' &&
                    i < MAX_CONFIG_LABEL_LEN - 1) {
            label [i++] = ch;
        }
        label [i] = '\0';

    } while (ch != ']');
}



/*
    Create plugins
    -------
    - state->plugins

        [ plugins ]
        Bld: b : (b)uild
        Src: f : source (f)ile
*/
static void create_plugins (FILE *file, state_t *state)
{
    int ch, i;
    plugin_t* curr_plugin = NULL;
    bool head_plugin_exists = false;
    bool title_started;

    while ((ch = fgetc (file)) != '[' && ch != EOF) {

        // if letter
        if (isalpha (ch)) {

            // allocate plugin
            if (head_plugin_exists) {
                curr_plugin->next = allocate_plugin ();
                curr_plugin = curr_plugin->next;
            } else {
                curr_plugin = allocate_plugin();
                state->plugins = curr_plugin;
                head_plugin_exists = true;
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
            title_started = false;
            while ((ch = fgetc (file)) != '\n' && i < (MAX_CONFIG_LABEL_LEN - 1)) {
                if (isalpha (ch) || ch == '(' || title_started) {
                    curr_plugin->title[i++] = ch;
                    title_started = true;
                }
            }
            curr_plugin->title[i] = '\0';
        }
    }

    // unget '['
    ungetc (ch, file);
}



/*
    Create new layout
*/
static layout_t* create_layout (FILE* file,
                                state_t *state, 
                                char *label)
{
    int ch, next_ch;
    char *win_keys;
    char section = 0;
    int num_chars = 0;
    bool is_key = false;
    char keys [MAX_KEY_STR_LEN] = {0};
    int i = 0;
    layout_t* layout = NULL;

    // allocate layout
    layout = allocate_layout ();
    layout->next = NULL;

    // add layout label
    strncpy (layout->label, label, strlen (label));

    // parse
    while ((ch = fgetc(file)) != '[' && ch != EOF) {

        // set section (h, w)
        if (ch == '>') {
            next_ch = fgetc (file);
            switch (next_ch) {
                case 'h':
                    section = 'h';
                    break;
                case 'w':
                    section = 'w';
                    break;
            }
        }

        // parse header or window section
        if (section == 'w' || section == 'h') {

            i = 0;
            num_chars = 0;

            // zero out number of header rows
            if (section == 'h')
                layout->num_hdr_key_rows = 0;

            // create header/window key string
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
                strncpy ((char *)layout->win_key_str, keys, num_chars + 1);
                win_keys = layout->win_key_str;
            }

            // unget '>' or '['
            ungetc (ch, file);

            num_chars = 0;
        }
        section = '\0';
    }

    // unget '>' or '['
    ungetc (ch, file);

    // Calculate window segment ratio
    // --------
    // A "segment" refers to the space needed for each window symbol character
    // in terms of the terminal's rows and columns. The following calculations
    // remain the same regardless of the current screen/pane's dimensions. They
    // are used by render_layout() to create the header and windows.
    //     
    //     ssbb
    //     ssww
    //     ccrr --> The 's' window will take up half of the screen's
    //              columns (2/4 segments) and two thirds of the rows
    //              (2/3 segments).
    // 
    // Here we are calculating the total ratio for all the keys.
    //
    //     * * *     
    //     * * * --> layout->col_ratio == 3
    //               layout->row_ratio == 2
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
    layout->row_ratio = y_ratio;
    layout->col_ratio = x_ratio;


    // allocate window matrix
    //
    //     {{s,s,b},
    //      {s,s,w},
    //      {c,c,r}}
    //
    char **layout_matrix = (char **) malloc (y_ratio * sizeof (char*));
    for (i = 0; i < y_ratio; i++) {
        layout_matrix [i] = (char *) malloc (x_ratio * sizeof (char));
    }
    if (layout_matrix == NULL) {
        pfeme ("Unable to create layout_matrix for %s\n", 
                    layout->label);
    }

    // add keys to matrix
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

    // add to layout
    layout->win_matrix = (char *) layout_matrix;

    return layout;
}



/*
    Allocate memory for layout_t struct
*/
static layout_t* allocate_layout (void)
{
    layout_t *layout = (layout_t*) malloc (sizeof (layout_t));
    if (layout == NULL) {
        pfeme ("layout_t allocation failed\n");
    }
    return layout;
}



/*
    Allocate memory for plugin_t struct
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



