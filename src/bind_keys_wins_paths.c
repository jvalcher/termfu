#include <string.h>

#include "data.h"
#include "utilities.h"
#include "plugins/_plugins.h"
#include "bind_keys_wins_paths.h"

static win_keys_t  *allocate_win_keys         (void);
static int          find_plugin_index         (plugin_t*);
static void         set_plugin_index          (int, char);
static void         add_win_ptr_and_out_path  (plugin_t*, window_t*);
static void         add_win_path              (int, plugin_t*);
static void         set_win_keys              (plugin_t*, win_keys_t*);

#ifdef DEBUG
static void   print_key_and_func_index (void);
#endif

#define ARR_SIZE  53

int key_function_index [ARR_SIZE] = {0};



/*
    Bind shortcut keys, window_t structs, window data file paths to plugin_t structs
*/
void bind_keys_wins_paths (state_t *state)
{
    int plugin_index;
    char key;
    plugin_t   *curr_plugin;

    state->win_keys = allocate_win_keys ();

    curr_plugin = state->plugins;
    do {

        key = curr_plugin->key;

        // find index of plugin_code[] matching current plugin's code,
        plugin_index = find_plugin_index (curr_plugin);

        // Convert key to index in key_function_index[index], set to plugin_index
        //
        //      {0,a-z,A-Z}  -->  {0-52}
        //
        set_plugin_index (plugin_index, key);

        // Add window_t pointer and debugger output data file path
        add_win_ptr_and_out_path (curr_plugin, state->curr_layout->windows);

        // Assign plugin key to win_keys_t if applicable
        set_win_keys (curr_plugin, state->win_keys);

        // add window output path if applicable
        add_win_path (plugin_index, curr_plugin);

        curr_plugin = curr_plugin->next;
    } while (curr_plugin != NULL);


#ifdef DEBUG
    print_key_and_func_index ();
#endif
}



static win_keys_t *allocate_win_keys (void)
{
    win_keys_t *win_keys = (win_keys_t*) malloc (sizeof (win_keys_t));
    if (win_keys == NULL) {
        pfeme ("win_cmds_t allocation error\n");
    }
    return win_keys;
}



static int find_plugin_index (plugin_t *curr_plugin)
{
    extern int plugin_code_size;    // plugins/_plugins.c

    int  start_index = 1;            // skip "EMP"
    int  mid_index;
    int  end_index = (plugin_code_size / sizeof(plugin_code[0])) - 1;
    int  cmp;
    int  plugin_index = -1;

    while (start_index <= end_index) {
        mid_index = start_index + (end_index - start_index) / 2;
        cmp = strcmp (plugin_code [mid_index], curr_plugin->code);
        if (cmp == 0) {
            plugin_index = mid_index;
            goto index_found;
        } else if (cmp < 0) {
            start_index = mid_index + 1;
        } else {
            end_index = mid_index - 1;
        }
    }
    index_found:

    if (plugin_index == -1)
        pfeme ("Unknown plugin code \"%s\"\n", curr_plugin->code);

    return plugin_index;
}



static void set_plugin_index (int plugin_index,
                              char key)
{
    if (key >= 'a' && key <= 'z') {
        key_function_index [key - 'a' + 1] = plugin_index;
    }
    else if (key >= 'A' && key <= 'Z') {
        key_function_index [key - 'A' + 27] = plugin_index;
    }
    else {
        pfeme ("\'%c\' key not found \n", key);
    }
}



static void add_win_ptr_and_out_path (plugin_t *curr_plugin,
                                      window_t *windows)
{
    window_t *curr_win = windows;
    curr_plugin->window = NULL;

    do {
        if (curr_plugin->key == curr_win->key) {

            // window_t
            curr_plugin->window = curr_win;

            break;
        }
        curr_win = curr_win->next;
    } while (curr_win != NULL);
}



static void add_win_path (int plugin_index, 
                          plugin_t *curr_plugin)
{
    char  path [256];
    char *home = getenv ("HOME");

    // data window
    if (curr_plugin->window) {
        snprintf (path, sizeof (path), "%s/%s/%s", home, DATA_DIR_PATH, win_file_name [plugin_index]);
        curr_plugin->window->out_file_path = (char*) malloc (strlen (path) + 1);
        strncpy (curr_plugin->window->out_file_path, path, strlen (path) + 1);
    } 
}



static void set_win_keys (plugin_t *curr_plugin,
                          win_keys_t *win_keys)
{
    if (strcmp (curr_plugin->code, "Bak") == 0) {
        win_keys->back = (int) curr_plugin->key;
    } 
    else if (strcmp (curr_plugin->code, "Qut") == 0) {
        win_keys->quit = (int) curr_plugin->key;
    } 
    else if (strcmp (curr_plugin->code, "ScU") == 0) {
        win_keys->scroll_up = (int) curr_plugin->key;
    } 
    else if (strcmp (curr_plugin->code, "ScD") == 0) {
        win_keys->scroll_down = (int) curr_plugin->key;
    } 
    else if (strcmp (curr_plugin->code, "ScL") == 0) {
        win_keys->scroll_left = (int) curr_plugin->key;
    } 
    else if (strcmp (curr_plugin->code, "ScR") == 0) {
        win_keys->scroll_right = (int) curr_plugin->key;
    } 
}



#ifdef DEBUG

/*
    Print (key : function index)
*/
static void print_key_and_func_index (void)
{
    printf ("Key binding: function index\n");
    int ch, index = 1;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 13; j++) {

            // convert index to key shortcut
            if (index >= 1 && index <= 26)
                ch = index + 'a' - 1;
            else if (index >= 27 && index <= 52)
                ch = index + 'A' - 27;

            printf ("%c:%d ", ch, key_function_index[index++]);
        }
    }
    printf ("\n\n");
}

#endif

