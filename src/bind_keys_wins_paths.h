
#ifndef BIND_KEYS
#define BIND_KEYS

#include "data.h"


/****************
  # Key bindings
 ****************/

/*
    Shortcut keys bound to plugin function indexes
    -------------
    - Set in main.c with bind_keys_to_plugins()  (below)
    - Used in run_plugin.c
*/
extern int key_function_index [];


/*
    Bind shortcut keys, window_t structs to plugin_t structs
    ------------
    - Called in main.c
    - Set function index in key_function_index[]

        {0,a-z,A-Z}  -->  {0-52}

        key  == 'c'    (index 3 in key_function_index {0,a,b,c,...} )
        code == 'Con'  (index 5 in plugin_code[])   (plugins/_plugins.c)

        key_function_index [3] = 5

        if  key_function_index [6] = 0   -->  unassigned

    - Key converted to function index by key_to_index() in run_plugin.c
*/
void bind_keys_wins_paths (state_t *state);




#endif

