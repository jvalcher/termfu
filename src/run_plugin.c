
/*
    Run plugin
*/

#include <stdio.h>
#include <string.h>

#include "run_plugin.h"
#include "plugins/termide.h"
#include "plugins/gdb.h"
#include "data.h"
#include "utilities.h"


/*
    Function pointer array
    ----------
    Indexes match corresponding plugin code string in plugin_index[]  (data.h)
*/
plugin_func_t plugin[] = {

    (plugin_func_t) empty_func,         // 0  "EMP"
    (plugin_func_t) gdb_assembly,       // 1  "Asm"
    (plugin_func_t) termide_back,       // 2  "Bak"
    (plugin_func_t) termide_builds,     // 3  "Bld"
    (plugin_func_t) gdb_breakpoints,    // 4  "Brk"
    (plugin_func_t) gdb_continue,       // 5  "Con"
    (plugin_func_t) gdb_finish,         // 6  "Fin"
    (plugin_func_t) gdb_kill,           // 7  "Kil"
    (plugin_func_t) termide_layouts,    // 8  "Lay"
    (plugin_func_t) gdb_local_vars,     // 9  "LcV"
    (plugin_func_t) gdb_next,           // 10 "Nxt"
    (plugin_func_t) gdb_output,         // 11 "Out"
    (plugin_func_t) gdb_prompt,         // 12 "Prm"
    (plugin_func_t) gdb_print,          // 13 "Prn"
    (plugin_func_t) gdb_registers,      // 14 "Reg"
    (plugin_func_t) gdb_run,            // 15 "Run"
    (plugin_func_t) gdb_src_file,       // 16 "Src"
    (plugin_func_t) gdb_step,           // 17 "Stp"
    (plugin_func_t) gdb_watches         // 18 "Wat"
};


int get_plugin_index (char *code);



/*
    Run plugin
    --------
    Match key input to plugin code as set in CONFIG_FILE (see data.h)
    Run plugin function

        input_key   - key pressed
        li          - layouts_t struct index
        layouts     - layouts_t struct

*/
/*
int run_plugin (char input_key, 
                int li, 
                layouts_t *layouts)
{
    int    i;
    int    num_lay_plugins  = layouts->plugins[li]->num;
    char (*layout_keys)[1]  = layouts->plugins[li]->key;
    char (*layout_codes)[4] = layouts->plugins[li]->code;
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
*/



