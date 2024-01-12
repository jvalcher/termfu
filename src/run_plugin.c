
/*
    Run plugin
*/

#include <stdio.h>
#include <string.h>

#include "run_plugin.h"
#include "data.h"
#include "utilities.h"

#include "plugins/termide.h"
#include "plugins/gdb.h"
/*
    Function pointer array
    ----------
    - Indexes match corresponding plugin code string in plugin_code[]  (render_layout.c)
        - Ordered alphabetically by plugin code
    - Indexes set in key_function_index[]  (data.h)  by bind_keys_to_plugins()  (render_layout.c)
    - Functions called in main() loop  (main.c)
*/
typedef int (*plugin_func_t) (layout_t *layout);
    //
plugin_func_t plugin[] = {

    (plugin_func_t) empty_func,         // "EMP"
    (plugin_func_t) gdb_assembly,       // "Asm"
    (plugin_func_t) termide_back,       // "Bak"
    (plugin_func_t) termide_builds,     // "Bld"
    (plugin_func_t) gdb_breakpoints,    // "Brk"
    (plugin_func_t) gdb_continue,       // "Con"
    (plugin_func_t) gdb_finish,         // "Fin"
    (plugin_func_t) gdb_kill,           // "Kil"
    (plugin_func_t) termide_layouts,    // "Lay"
    (plugin_func_t) gdb_local_vars,     // "LcV"
    (plugin_func_t) gdb_next,           // "Nxt"
    (plugin_func_t) gdb_output,         // "Out"
    (plugin_func_t) gdb_prompt,         // "Prm"
    (plugin_func_t) gdb_print,          // "Prn"
    (plugin_func_t) gdb_registers,      // "Reg"
    (plugin_func_t) gdb_run,            // "Run"
    (plugin_func_t) gdb_src_file,       // "Src"
    (plugin_func_t) gdb_step,           // "Stp"
    (plugin_func_t) gdb_watches         // "Wat"
};



/*
    Run plugin
    --------
    Match key input to plugin code as set in CONFIG_FILE (see data.h)
    Run plugin function

        input_key   - key pressed

*/
int run_plugin (int input_key,
                layout_t *layout)
{
    int plugin_index;
    int function_index;

    // get plugin function index
    plugin_index = key_to_index (input_key);
    function_index = key_function_index [plugin_index];

    // run plugin
    if (plugin[function_index](layout) == -1) {
        pfeme ("Unable to run function index %d with key \"%c\"\n", 
                function_index, input_key);
    }

    return 1;
}
