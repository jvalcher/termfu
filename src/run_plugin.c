
/*
    Run plugin
*/

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "run_plugin.h"
#include "data.h"
#include "bind_keys_wins_paths.h"
#include "plugins/_plugins.h"
#include "render_window_data.h"

static int       key_to_index               (int);
static plugin_t *get_plugin                 (int, plugin_t*);




/*
    Run plugin function
    --------
    - Match key input to plugin code in current layout
    - Indicate usage via the plugin's title string in the header or its window
    - Run plugin code's function via its pointer
*/
void 
run_plugin (int      key,
            state_t *state)
{
    int plugin_index   = key_to_index (key);

    if (plugin_index != -1) {
        
        int func_index     = key_function_index [plugin_index];
        state->curr_plugin = get_plugin (key, state->plugins);
        state->curr_window = state->curr_plugin->window;

        plugin_func [func_index] (state);

        //sem_wait (state->reader_sem);

        //sem_post (state->reader_sem);
    }
}



/* 
    Convert key letter to plugin function index
*/
static int
key_to_index (int key)
{
    if (key >= 'a' && key <= 'z') {
        return key - 'a' + 1;
    }
    else if (key >= 'A' && key <= 'Z') {
        return key - 'A' + 27;
    } else {
        return -1;
    }
}



/*
    Get plugin associated with input key
*/
static plugin_t*
get_plugin (int key,
            plugin_t *plugins)
{
    plugin_t *curr_plugin = plugins;
    do {
        if (key == curr_plugin->key) {
            return curr_plugin;
        }
        curr_plugin = curr_plugin->next;
    } while (curr_plugin != NULL);

    return NULL;
}



