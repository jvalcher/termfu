
#include "_plugins.h"
#include "termide.h"
#include "gdb.h"


/*
    Plugin codes
    -----------
    - Indexes match corresponding function's index in plugin_function[] array  (run_plugin.c)
    - Ordered alphabetically for binary search in bind_keys_to_plugin()
    - Used to bind function index to shortcut key in key_function_index[]  (data.h)
*/
char *plugin_code [] = {
    
    "EMP",
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
    "Wat"
};
int plugin_code_size = sizeof (plugin_code);


/*
    Function pointer array
    ----------
    - Indexes match corresponding plugin code string in plugin_code[]
        - Ordered alphabetically by code
    - Indexes set by bind_keys_to_plugins()  (bind_keys_windows.c)
    - Functions called by run_plugin() in main() loop  (main.c)
*/
plugin_func_t plugin_func_arr[] = {

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

