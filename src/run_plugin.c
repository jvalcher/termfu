
/*
    Run plugin
*/

#include <stdio.h>
#include <string.h>

#include "data.h"
#include "utilities.h"
#include "plugins/_plugins.h"



/*
    Action codes
    -----
    Sorted, three-letter, case-sensitive global identifiers for plugins

    Key shortcut and title string associated with each plugin are 
    set in CONFIG_FILE for each layout

        See "Example layout configuration" in `_Readme.md`

    See src/plugins/_plugins.h for plugin information

*/
const char *plugin_code [] = {
    "Asm",
    "Bak",
    "Bld",
    "Brk",
    "Con",
    "Fin",
    "Kil",
    "Lay",
    "LcV",
    "Nxt",
    "Out",
    "Prm",
    "Prn",
    "Reg",
    "Run",
    "Src",
    "Stp",
    "Wat",
};
enum plugin_index {
    Asm,
    Bak,
    Bld,
    Brk,
    Con,
    Fin,
    Kil,
    Lay,
    LcV,
    Nxt,
    Out,
    Prm,
    Prn,
    Reg,
    Run,
    Src,
    Stp,
    Wat
};


int get_plugin_index (char *code);



/*
    Run plugin
    --------
    Match key input to plugin code as set in CONFIG_FILE (see data.h)

    Uses NUM_PLUGINS in src/data.h


        input_key   - key pressed
        li          - layouts_t struct index
        layouts     - layouts_t struct

*/
int run_plugin (char input_key, 
                int li, 
                layouts_t *layouts)
{
    int    i;
    int    num_lay_plugins  = layouts->plugins[li]->num;
    char (*layout_keys)[1]  = layouts->plugins[li]->keys;
    char (*layout_codes)[4] = layouts->plugins[li]->codes;
    char  *code             = NULL; 
    int    pi               = 0;
    int    result           = -1;

    // match input key to plugin code
    for (i = 0; i < num_lay_plugins; i++) {
        if (input_key == layout_keys[i][0]) {
            code = layout_codes[i];
        }
    }

    // get plugin_code index
    pi = get_plugin_index (code);

    // call function
    switch ((enum plugin_index) pi) {
        case Asm: result = gdb_assembly (); break;
        case Bak: result = termide_back (); break;
        case Bld: result = termide_builds (); break;
        case Brk: result = gdb_breakpoints (); break;
        case Con: result = gdb_continue (); break;
        case Fin: result = gdb_finish (); break;
        case Kil: result = gdb_kill (); break;
        case Lay: result = termide_layouts (); break;
        case LcV: result = gdb_local_vars (); break;
        case Nxt: result = gdb_next (); break;
        case Out: result = gdb_output (); break;
        case Prm: result = gdb_prompt (); break;
        case Prn: result = gdb_print (); break;
        case Reg: result = gdb_registers (); break;
        case Run: result = gdb_run (); break;
        case Src: result = gdb_src_file (); break;
        case Stp: result = gdb_step (); break;
        case Wat: result = gdb_watches (); break;
    }

    return result;
}



/*
    Get plugin function pointer index
    ------------
    Binary search for plugin code in `plugin_code` array
    Return index
        - matches index of related function pointer in `plugin` array
*/
int get_plugin_index (char *code)
{
    int start_index = 0;
    int end_index = NUM_PLUGINS - 1;
    int mid_index;
    int cmp;

    while (start_index <= end_index) {

        mid_index = start_index + (end_index - start_index) / 2;
        cmp = strcmp (plugin_code[mid_index], code);

        if (cmp == 0) {
            return mid_index;
        } else if (cmp < 0) {
            start_index = mid_index + 1;
        } else {
            end_index = mid_index - 1;
        }
    }

    // plugin string not found
    endwin ();
    pfem ("Unknown plugin code \"%s\"\n", code);
    exit (EXIT_FAILURE);
}

