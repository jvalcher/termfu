
/*
    Parse configuration file
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <ncurses.h>

#include "parse_config_file.h"
#include "data.h"
#include "plugins.h"
#include "utilities.h"

static char      **create_command    (FILE*, state_t*);
static void        create_plugins    (FILE*, state_t*);
static layout_t   *create_layout     (FILE*, char*);
static void        allocate_plugins  (state_t*);
static layout_t   *allocate_layout   (void);

extern char **plugin_codes;
extern char **win_file_names;

// indexes matches enum in data.h
// { DEBUGGER_GDB }
char *debuggers[] = { "gdb" };



/*
    Parse CONFIG_FILE data
    --------------
    - Allocate and add plugins to `plugin` *plugin_t array  (plugins.h)
    - Create, return layout_t linked list
*/
void
parse_config_file (state_t *state)
{
    FILE     *fp;
    int       num_keys;
    bool      is_first_layout = true;
    layout_t *prev_layout = NULL,
             *curr_layout = NULL,
             *head_layout = NULL;
    char      ch,
              category [MAX_CONFIG_CATEG_LEN] = {'\0'},
              label    [MAX_CONFIG_LABEL_LEN] = {'\0'};

    num_keys = (int)'z' + 1;
    state->plugin_key_index = (int*) malloc (num_keys * sizeof (int));

    set_num_plugins (state);

    allocate_plugins (state);

    allocate_plugin_windows (state);

    fp = open_config_file ();
    if (fp == NULL) {
        pfeme ("Failed to open configuration file\n");
    }

    // parse config file
    is_first_layout = true;
    while ((ch = fgetc (fp)) != EOF) {

        // TODO: add newline, inline #comment functionality

        if (ch == '[') {
            get_category_and_label (fp, category, label);
        }

        // create state->command
        if (strcmp (category, CONFIG_COMMAND_LABEL) == 0) {
            state->command = create_command (fp, state);
        }

        // create state->plugins
        if (strcmp (category, CONFIG_PLUGINS_LABEL) == 0) {
            create_plugins (fp, state);
        }

        // create linked list of state->layouts
        if (strcmp (category, CONFIG_LAYOUTS_LABEL) == 0) {
            curr_layout = create_layout (fp, label);
            if (is_first_layout) {
                head_layout = curr_layout;
                is_first_layout = false;
            } else {
                prev_layout->next = curr_layout;
                curr_layout->next = NULL;
            }
            prev_layout = curr_layout;
        }

        category [0] = '\0';
        label [0] = '\0';
    }

    fclose (fp);

    state->layouts = head_layout;
    state->curr_layout = NULL;
}


static void
allocate_plugins (state_t *state)
{
    state->plugins = (plugin_t**) malloc (state->num_plugins * sizeof (plugin_t*));
    if (state->plugins == NULL) {
        pfeme ("plugin_t pointer array allocation failed\n");
    }
    for (int i = 0; i < state->num_plugins; i++) {
        state->plugins [i] = (plugin_t*) malloc (sizeof (plugin_t));
        if (state->plugins [i] == NULL) {
            pfeme ("plugin_t pointer allocation failed\n");
        }
    }
}



static char**
create_command (FILE *fp,
                state_t *state)
{
    char **cmd_arr,
           buff [48];
    int ch, n, i,
        num_debuggers;
    long save_fp;
    bool first_word,
         debugger_supported;
    
    // count words
    do {
        ch = fgetc (fp);
    } while (!isalpha (ch));
        //
    ungetc (ch, fp);
    save_fp = ftell (fp);
    getc (fp);
        //
    n = 1;  
    do {
        if (ch == ' ') {
            ++n;
        }
    } while ((ch = fgetc (fp)) != '\n');
    ++n;    // execvp NULL

    cmd_arr = (char**) malloc (n * sizeof (char*));


    // create array
    fseek (fp, save_fp, SEEK_SET);
    n = 0;  
    i = 0;
    first_word = true;
    debugger_supported = false;
    num_debuggers = sizeof (debuggers) / sizeof (debuggers [0]);

    while ((ch = fgetc (fp)) != '\n' && ch != EOF) {

        if (ch != ' ') {

            // get word
            i = 0;
            do {
                buff [i++] = ch;
            } while ((ch = fgetc (fp)) != ' ' && ch != '\n');
            buff [i] = '\0';

            logd ("%s\n", buff);

            // check if debugger supported
            if (first_word) {
                for (i = 0; i < num_debuggers; i++) {
                    if (strcmp (debuggers [i], buff) == 0) {
                        state->debugger->curr = i;
                        debugger_supported = true;
                    }
                }
                if (debugger_supported == false) {
                    pfeme ("Unrecognized debugger \"%s\"\n", buff);
                }
                first_word = false;
            }

            cmd_arr [n] = (char*) malloc (strlen (buff) + 1);
            strcpy (cmd_arr [n++], buff); 
        }

    }

    cmd_arr [n] = NULL;

    return cmd_arr;
}



static layout_t*
allocate_layout (void)
{
    layout_t *layout = (layout_t*) malloc (sizeof (layout_t));
    if (layout == NULL) {
        pfeme ("layout_t allocation failed\n");
    }
    return layout;
}



/*
    Create plugins
    -------
    - Allocate and add to `plugin` array in plugins.h

        [ plugins ]
        Lay: l : (l)ayout
        Src: f : source (f)ile
*/
static void create_plugins (FILE *file, state_t *state)
{
    int       i, j,
              key,
              plugin_index;
    char      title [MAX_TITLE_LEN],
              code  [PLUGIN_CODE_LEN + 1];
    bool      title_started;
    plugin_t *curr_plugin = NULL;

    while ((key = fgetc (file)) != '[' && key != EOF) {

        // if letter
        if (isalpha (key)) {

            // get code
            for (j = 0; j < PLUGIN_CODE_LEN; j++) {
                code[j] = key;
                key = fgetc (file);
            }
            code [PLUGIN_CODE_LEN] = '\0';

            plugin_index = get_plugin_code_index (code, state);
            curr_plugin = state->plugins [plugin_index];
            strcpy (curr_plugin->code, code);

            // set key
            ungetc (key, file);
            while ((key = fgetc (file)) != ':') {;}
            while ((key = fgetc (file)) != ':') {
                if (isalpha (key)) {
                    curr_plugin->key = key;
                }
            }

            // set plugin_key_index
            state->plugin_key_index [(int) curr_plugin->key] = plugin_index;

            // set title
            i = 0;
            title_started = false;
            while ((key = fgetc (file)) != '\n' && i < (MAX_CONFIG_LABEL_LEN - 1)) {
                if (isalpha (key) || key == '(' || title_started) {
                    title[i++] = key;
                    title_started = true;
                }
            }
            title[i] = '\0';
            curr_plugin->title = (char*) malloc (strlen (title) + 1);
            strncpy (curr_plugin->title, title, strlen(title) + 1);
        }
    }

    // unget '[', EOF
    ungetc (key, file);
}



/*
    Create new layout
*/
static layout_t*
create_layout (FILE* file,
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
    // are used by render_layout() to create the Ncurses header and windows.
    //     
    //     ssbb
    //     ssww
    //     ccrr --> The 's' window will take up half of the screen's
    //              columns (2/4 column segments) and two thirds of the rows
    //              (2/3 row segments).
    // 

    // Calculate total ratio for all keys
    //
    //     ****     
    //     ****     
    //     **** --> col_ratio == 4
    //              row_ratio == 3
    //
    int y_ratio   = 0;
    int x_ratio   = 0;
    bool  x_count = true;
    for (size_t m = 0; m < strlen (win_keys); m++) {
        if (win_keys [m] == '\n') {
            y_ratio += 1;
            x_count = false;
        }
        if (x_count == true && isalpha(win_keys[m])) {
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
    for (size_t m = 0; m < strlen (win_keys); m++) {
        layout_matrix [row][col] = win_keys [m];
        if (col < x_ratio - 1) {
            col += 1;
        } else {
            col  = 0;
            row += 1;
            m   += 1;     // skip '\n'
        }
    }

    layout->win_matrix = layout_matrix;

    return layout;
}

