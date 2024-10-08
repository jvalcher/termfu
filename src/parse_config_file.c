
/*
    Parse configuration file
*/

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <ncurses.h>
#include <errno.h>

#include "parse_config_file.h"
#include "data.h"
#include "plugins.h"
#include "utilities.h"

static int         allocate_plugins       (state_t*);
static FILE*       open_config_file       (state_t*);
static int         get_category_and_label (FILE *file, char *category, char *label);
static char      **create_command         (FILE*, state_t*);
static int         create_plugins         (FILE*, state_t*);
static layout_t   *create_layout          (FILE*, char*);

extern char **plugin_codes;
extern char **win_file_names;

// indexes match enums in data.h  { DEBUGGER_GDB, DEBUGGER_PDB}
char *debuggers[] = { "gdb", "pdb" };



int
parse_config_file (state_t *state)
{
    FILE     *fp;
    int       key_arr_len,
              ret;
    bool      is_first_layout = true;
    layout_t *prev_layout = NULL,
             *curr_layout = NULL,
             *head_layout = NULL;
    char      ch,
              category [LAYOUT_CATEG_LEN] = {'\0'},
              label    [LAYOUT_LABEL_LEN] = {'\0'};

    // state->plugin_key_index
    key_arr_len = (int)'z' + 1;
    if ((state->plugin_key_index = (int*) malloc (key_arr_len * sizeof (int))) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Failed to allocate state->plugin_key_index");
    }

    // state->num_plugins
    set_num_plugins (state);

    // state->plugins
    ret = allocate_plugins (state);
    if (ret == FAIL) {
        pfemr ("Failed to allocate plugins");
    }

    // state->plugins[x]->win
    ret = allocate_plugin_windows (state);
    if (ret == FAIL) {
        pfemr ("Failed to allocate plugin windows");
    }

    if ((fp = open_config_file (state)) == NULL) {
        pfemr ("Failed to open configuration file");
    }

    // parse config file
    is_first_layout = true;
    while ((ch = fgetc (fp)) != EOF) {

        // newline comment
        if (ch == ('#')) {
            while ((ch = fgetc (fp)) != ('\n')) {
                ;
            }
        }

        if (ch == '[') {
            ret = get_category_and_label (fp, category, label);
            if (ret == FAIL) {
                pfemr ("Failed to get category and label");
            }
        }

        // create state->command
        if (strcmp (category, CONFIG_COMMAND_LABEL) == 0) {
            if ((state->command = create_command (fp, state)) == NULL) {
                pfemr ("Failed to create command");
            }
        }

        // create state->plugins
        if (strcmp (category, CONFIG_PLUGINS_LABEL) == 0) {
            ret = create_plugins (fp, state);
            if (ret == FAIL) {
                pfemr ("Failed to create plugins");
            }
        }

        // create linked list of state->layouts
        if (strcmp (category, CONFIG_LAYOUTS_LABEL) == 0) {

            if ((curr_layout = create_layout (fp, label)) == NULL) {
                pfemr ("Failed to create layout \"%s\"", label);
            }

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

    return A_OK;
}



static int
allocate_plugins (state_t *state)
{
    // state->plugins
    if ((state->plugins = (plugin_t**) malloc (state->num_plugins * sizeof (plugin_t*))) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Failed to allocate plugin array for %d plugins", state->num_plugins);
    }

    // state->plugins[i]
    for (int i = 0; i < state->num_plugins; i++) {
        if ((state->plugins [i] = (plugin_t*) malloc (sizeof (plugin_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("plugin_t pointer allocation failed (index: %d, code: %s)", i, get_plugin_code (i));
        }
    }

    return A_OK;
}



static FILE*
open_config_file (state_t *state)
{
    FILE *file = NULL;
    if (state->config_path[0] != '\0') {
        if ((file = fopen (state->config_path, "r")) == NULL) {
            pfem ("fopen error: %s", strerror (errno));
            pem ("fopen error for config file \"./%s\": %s", state->config_path, strerror (errno));
        }
    } else {
        if ((file = fopen (CONFIG_FILE, "r")) == NULL) {
            pfem ("fopen error: %s", strerror (errno));
            pfem ("fopen error for config file \"./%s\": %s", CONFIG_FILE, strerror (errno));
        }
    } 
    return file;
}



/*
    Get category and label
    -------
        [ <categ> : <label> ]
*/
static int
get_category_and_label (FILE *file,
                        char *category,
                        char *label)
{
    int  i,
         ch = 0;
    long start;

    // save current file position
    if ((start = ftell (file)) == -1) {
        ;
    }

    do {
        // category
        i = 0;
        while ((ch = fgetc (file)) != ':'  &&
                                ch != ']'  &&
                                ch != '\n' &&
                                 i <  LAYOUT_CATEG_LEN - 1) {
            if (ch != ' ')
                category [i++] = ch;
        }
        category [i] = '\0'; 

        // if no label, break
        if (ch == ']') {
            label [0] = '\0';
            break;
        }

        // if newline reached, error
        if (ch == '\n') {
            pfem ("Newline character reached, formatting error");
            pem  ("\"[");
            while ((ch = fgetc (file)) != '\n') {
                fputc (ch, stderr);
            }
            pemr ("\"");
        }

        // label
        i = 0;
        while ((ch = fgetc (file)) != ']' &&
                    i < LAYOUT_LABEL_LEN - 1) {
            label [i++] = ch;
        }
        label [i] = '\0';

    } while (ch != ']');

    return A_OK;
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
    bool debugger_supported;
    
    // count words
    do {
        ch = fgetc (fp);
        if (ch == ('#')) {
            while ((ch = fgetc (fp)) != ('\n')) {
                ;
            }
        }
    } while (!isalpha (ch));

    ungetc (ch, fp);
    save_fp = ftell (fp);
    getc (fp);

    n = 1;  
    do {
        if (ch == ' ') {
            ++n;
        }
    } while ((ch = fgetc (fp)) != '\n');
    ++n;    // execvp NULL

    if ((cmd_arr = (char**) malloc (n * sizeof (char*))) == NULL) {
        pfem ("malloc: %s\n", strerror (errno));
        pem  ("Failed to allocate cmd_arr (n = %d): %s", n, strerror (errno));
        return NULL;
    }

    // create array
    fseek (fp, save_fp, SEEK_SET);
    n = 0;  
    i = 0;
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
            if (ch == '\n') {
                ungetc (ch, fp);
            }

            // check if word == supported debugger
            if (debugger_supported == false) {

                for (i = 0; i < num_debuggers; i++) {

                    // set state->debugger->index and ->title
                    if (strcmp (debuggers [i], buff) == 0) {
                        state->debugger->index = i;
                        memcpy (state->debugger->title, debuggers [i], DEBUG_TITLE_LEN - 1);
                        state->debugger->title [DEBUG_TITLE_LEN - 1] = '\0';
                        debugger_supported = true;
                    }
                }
            }

            if ((cmd_arr [n] = (char*) malloc (strlen (buff) + 1)) == NULL) {
                pfem ("malloc: %s", strerror (errno));
                pem  ("Failed to allocate cmd_arr element \"%s\": %s", buff, strerror (errno));
                return NULL;
            }
            strcpy (cmd_arr [n++], buff); 
        }
    }

    // debugger not supported
    if (debugger_supported == false) {
        pfem ("Debugger not supported -- See README.md for more information.\n\n");
        return NULL;
    }

    // execvp final command element
    cmd_arr [n] = NULL;

    return cmd_arr;
}



/*
    Create plugins
    -------
    - Allocate and add plugins to state->plugins

        [ plugins ]
        Lay: l : (l)ayout
        Src: f : source (f)ile
*/
static int
create_plugins (FILE *file, state_t *state)
{
    int       i, j,
              key,
              plugin_index;
    char      title [LAYOUT_TITLE_LEN],
              code  [PLUGIN_CODE_LEN + 1];
    bool      title_started;
    plugin_t *curr_plugin = NULL;

    while ((key = fgetc (file)) != '[' && key != EOF) {

        if (key == ('#')) {
            while ((key = fgetc (file)) != ('\n')) {
                ;
            }
        }

        if (isalpha (key)) {

            // plugin code
            for (j = 0; j < PLUGIN_CODE_LEN; j++) {
                code[j] = key;
                key = fgetc (file);
            }
            code [PLUGIN_CODE_LEN] = '\0';

            // get state->plugins [plugin_index]
            if ((plugin_index = get_plugin_code_index (code, state)) == FAIL) {
                pfemr ("Failed to find plugin index");
            }
            curr_plugin = state->plugins [plugin_index];

            // set plugin code
            memcpy (curr_plugin->code, code, PLUGIN_CODE_LEN + 1);
            curr_plugin->code [PLUGIN_CODE_LEN - 1] = '\0';

            // set key binding
            ungetc (key, file);
            while ((key = fgetc (file)) != ':') {
                ;
            }
            while ((key = fgetc (file)) != ':') {
                if (isalpha (key)) {
                    curr_plugin->key = key;
                }
            }

            // set state->plugin_key_index
                // So that in main() loop:
                // if key pressed == 'c'  -->  plugin_key_index [key] == plugin index bound to 'c'
            state->plugin_key_index [(int) curr_plugin->key] = plugin_index;

            // title
            i = 0;
            title_started = false;
            while ((key = fgetc (file)) != '\n' && i < (LAYOUT_LABEL_LEN - 1)) {
                if (isalpha (key) || key == '(' || title_started) {
                    title[i++] = key;
                    title_started = true;
                }
            }
            title[i] = '\0';

            // set curr_plugin->title
            if ((curr_plugin->title = (char*) malloc (strlen (title) + 1)) == NULL) {
                pfem ("malloc: %s", strerror (errno));
                pemr ("Failed to allocate title \"%s\" for code %s, key %c",
                            title, code, key);
            }
            strcpy (curr_plugin->title, title);
        }
    }

    // unget '[', EOF
    ungetc (key, file);

    return A_OK;
}



/*
    Create new layout
*/
static layout_t*
create_layout (FILE* file,
               char *label)
{
    int        ch,
               section_ch,
               num_chars,
               i;
    char       keys [LAYOUT_KEY_STR_LEN],
              *win_keys,
             **layout_matrix;
    bool       is_key = false;
    layout_t  *layout = NULL;

    // allocate layout
    if ((layout = (layout_t*) malloc (sizeof (layout_t))) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pem  ("layout_t allocation failed for layout \"%s\"\n", label);
        return NULL;
    }
    layout->next = NULL;

    // add layout label
    memcpy (layout->label, label, LAYOUT_LABEL_LEN - 1);
    layout->label [LAYOUT_LABEL_LEN - 1] = '\0';

    // parse
    while ((ch = fgetc(file)) != '[' && ch != EOF) {

        if (ch == ('#')) {
            while ((ch = fgetc (file)) != ('\n')) {
                ;
            }
        }

        // section h, w
        else if (ch == '>') {

            section_ch = fgetc (file);

            // parse header or window section
            if (section_ch == 'w' || section_ch == 'h') {

                i = 0;
                num_chars = 0;

                if (section_ch == 'h')
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
                        ch != '#' &&
                        ch != EOF &&
                        num_chars <= LAYOUT_KEY_STR_LEN) {

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

                        if (section_ch == 'h')
                            layout->num_hdr_key_rows += 1;

                        keys [i++] = ch;
                        num_chars += 1;
                    } 
                }
                keys [i] = '\0';

                // add string to layout
                if (section_ch == 'h') {
                    memcpy (layout->hdr_key_str, keys, LAYOUT_KEY_STR_LEN - 1);
                    layout->hdr_key_str [LAYOUT_KEY_STR_LEN - 1] = '\0';
                } else {
                    memcpy (layout->win_key_str, keys, LAYOUT_KEY_STR_LEN - 1);
                    layout->win_key_str [LAYOUT_KEY_STR_LEN - 1] = '\0';
                    win_keys = layout->win_key_str;
                }

                // unget '>' or '['
                ungetc (ch, file);

                num_chars = 0;
            }
            section_ch = '\0';
        }

    }

    // unget '>' or '['
    ungetc (ch, file);

    // Calculate window segment ratio
    // --------
    // A "segment" refers to the space needed for each window key shortcut
    // in terms of the terminal's rows and columns. The following calculations
    // remain the same regardless of the current screen/pane's dimensions. They
    // are used by render_layout() to create the ncurses header and windows.
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
    if (y_ratio == 0 || x_ratio == 0) {
        pfem ("Ratio calculation error (y: %d, x: %d)", y_ratio, x_ratio);
        return NULL;
    }


    // allocate window matrix
    //
    //     {{s,s,b},
    //      {s,s,w},
    //      {c,c,r}}
    //
    if ((layout_matrix = (char**) malloc (y_ratio * sizeof (char*))) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pem  ("Failed to allocate layout_matrix");
        return NULL;
    }
    for (i = 0; i < y_ratio; i++) {
        if ((layout_matrix [i] = (char*) malloc (x_ratio * sizeof (char))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pem ("Failed to allocate layout_matrix element (index: %d, code: %s)",
                        i, get_plugin_code (i));
            return NULL;
        }
    }

    // add keys to layout_matrix
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

