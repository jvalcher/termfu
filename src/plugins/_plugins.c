/*

Plugin
------
- Each code case-sensitive, three-letter code is associated with a plugin function 
    - see plugins/_interface.c

- Codes:

    - Window

        Asm     Assembly code
        Brk     Breakpoints
        LcV     Local variables
        Out     Program output
        Prm     Debugger prompt, output
        Reg     Registers
        Src     Source file
        Wat     Watchpoints

    - Pop-up window

        Bld     Project build selection
        Lay     Layout selection

    - Non-window

        EMP     Empty function
        Bak     Back out of window
        Qut     Quit

        Con     Continue
        Fin     Finish
        Kil     Kill
        Nxt     Next
        Run     (Re)run program
        Stp     Step
        Unt     Until

    - Misc
        ScU     Scroll window up
        ScD     Scroll window down
        ScL     Scroll window left
        ScR     Scroll window right
*/

#include "_plugins.h"
#include "_interface.h"

#define FILE_PATH  "/var/lib/termide/"



/*
    Plugin codes
    -----------
    - Indexes match corresponding indexes in:
        - plugin_function []
        - win_file_path []

    - Used in bind_keys_windows_to_plugins()
        - Ordered alphabetically for binary search  (A-Z,a-z)
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
    "Qut",
    "Reg",
    "Run",
    "ScD",
    "ScL",
    "ScR",
    "ScU",
    "Src",
    "Stp",
    "Unt",
    "Wat"
};
int plugin_code_size = sizeof (plugin_code);



/*
    Function pointer array
    ----------
    - Called by run_plugin()
*/
plugin_func_t plugin_func [] = {

    (plugin_func_t) empty_func,         // "EMP"

    (plugin_func_t) win_assembly,       // "Asm"
    (plugin_func_t) back,               // "Bak"
    (plugin_func_t) pwin_builds,        // "Bld"
    (plugin_func_t) win_breakpoints,    // "Brk"
    (plugin_func_t) cont,               // "Con"
    (plugin_func_t) finish,             // "Fin"
    (plugin_func_t) kill_prog,          // "Kil"
    (plugin_func_t) pwin_layouts,       // "Lay"
    (plugin_func_t) win_local_vars,     // "LcV"
    (plugin_func_t) next,               // "Nxt"
    (plugin_func_t) win_prog_output,    // "Out"
    (plugin_func_t) win_prompt,         // "Prm"
    (plugin_func_t) quit,               // "Qut"
    (plugin_func_t) win_registers,      // "Reg"
    (plugin_func_t) run,                // "Run"
    (plugin_func_t) scroll_down,        // "ScD"
    (plugin_func_t) scroll_left,        // "ScL"
    (plugin_func_t) scroll_right,       // "ScR"
    (plugin_func_t) scroll_up,          // "ScU"
    (plugin_func_t) win_src_file,       // "Src"
    (plugin_func_t) step,               // "Stp"
    (plugin_func_t) until,              // "Unt"
    (plugin_func_t) win_watches         // "Wat"
};



/*
    Window data file names
    ---------
    - Bound to plugins in bind_keys_windows_to_plugins()
*/
char *win_file_path [] = {

    NULL,                           // "EMP"
    FILE_PATH "assembly.asm",       // "Asm"
    NULL,                           // "Bak"
    FILE_PATH "builds.lst",         // "Bld"
    FILE_PATH "breakpoints.lst",    // "Brk"
    NULL,                           // "Qut"
    NULL,                           // "Con"
    NULL,                           // "Fin"
    NULL,                           // "Kil"
    FILE_PATH "layouts.lst",        // "Lay"
    FILE_PATH "local_vars.lst",     // "LcV"
    NULL,                           // "Nxt"
    FILE_PATH "program.out",        // "Out"
    FILE_PATH "debugger.out",       // "Prm"
    FILE_PATH "registers.lst",      // "Reg"
    NULL,                           // "Run"
    NULL,                           // "ScD"
    NULL,                           // "ScL"
    NULL,                           // "ScR"
    NULL,                           // "ScU"
    FILE_PATH "current.src",        // "Src"
    NULL,                           // "Stp"
    NULL,                           // "Unt"
    FILE_PATH "watchpoints.lst",    // "Wat"
};
