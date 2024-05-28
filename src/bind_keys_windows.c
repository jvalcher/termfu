#include <string.h>

#include "bind_keys_windows.h"
#include "data.h"
#include "utilities.h"
#include "plugins/_plugins.h"


int   key_function_index [53];


/*
    Bind shortcut keys, window_t structs to plugin_t structs
*/
void bind_keys_windows_to_plugins (layout_t* layout)
{
    int  start_index;
    int  end_index;
    int  mid_index;
    int  plugin_index;
    int  cmp;
    char key;
    char code[4];
    extern int plugin_code_size;    // plugins/_plugins.c
    plugin_t* curr_plugin;
    window_t* curr_window;

    curr_plugin = layout->plugins;
    do {

        // get shortuct key, code
        key = curr_plugin->key;
        strncpy (code, curr_plugin->code, strlen (plugin_code[0]) + 1);

        // find plugin_code[] index matching current plugin's code string
        start_index = 0;
        end_index = (plugin_code_size / sizeof(plugin_code[0])) - 1;
        plugin_index = -1;
            //
        while (start_index <= end_index) {
            mid_index = start_index + (end_index - start_index) / 2;
            cmp = strcmp (plugin_code [mid_index], code);
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

            // plugin code string not found
        if (plugin_index == -1)
            pfeme ("Unknown plugin code \"%s\"\n", code);

        // Set key_function_index[key] to plugin_index
        //
        //      {0,a-z,A-Z}  -->  {0-52}
        //
        if (key >= 'a' && key <= 'z') {
            key_function_index [key - 'a' + 1] = plugin_index;
        }
        else if (key >= 'A' && key <= 'Z') {
            key_function_index [key - 'A' + 27] = plugin_index;
        }
        else {
            pfeme ("\'%c\' key not found \n", key);
        }

        // add window_t pointer if available
        curr_plugin->window = NULL;
        curr_window = layout->windows;
        do {
            if (curr_plugin->key == curr_window->key) {
                curr_plugin->window = curr_window;
            }
            curr_window = curr_window->win_next;
        } while (curr_window != NULL);

        // next plugin
        curr_plugin = curr_plugin->next;

    } while (curr_plugin != NULL);
}



