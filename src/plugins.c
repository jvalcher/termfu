/*

Plugins
------
- Each case-sensitive, three-letter code is associated with a plugin
- Each plugin is assigned a key shortcut [A-Z,a-z] in the configuration file

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

#include <string.h>

#include "plugins.h"
#include "data.h"
#include "utilities.h"


/*
    Plugin codes
    -----------
    - Indexes match corresponding indexes in:
        - plugin_function []
        - win_file_path []

    - Used in bind_keys_windows_to_plugins()
        - Ordered alphabetically for binary search  (A-Z,a-z)
*/
char *plugin_codes [] = {
    
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



/*
    Window data file names
*/
char *win_file_name [] = {

    NULL,                       // "EMP"
    "",             // "Asm"
    NULL,                       // "Bak"
    "builds.lst",               // "Bld"
    "breakpoints.lst",          // "Brk"
    NULL,                       // "Con"
    NULL,                       // "Fin"
    NULL,                       // "Kil"
    "layouts.lst",              // "Lay"
    "local_vars.lst",           // "LcV"
    NULL,                       // "Nxt"
    "program.out",              // "Out"
    "debugger.out",             // "Prm"
    NULL,                       // "Qut"
    "registers.lst",            // "Reg"
    NULL,                       // "Run"
    NULL,                       // "ScD"
    NULL,                       // "ScL"
    NULL,                       // "ScR"
    NULL,                       // "ScU"
    "curr_src.file",            // "Src"    TODO: set path to actual source file
    NULL,                       // "Stp"
    NULL,                       // "Unt"
    "watchpoints.lst"           // "Wat"
};



void
set_num_plugins (state_t *state)
{
    state->num_plugins = sizeof (plugin_codes) / sizeof (plugin_codes [0]);

}



void
set_plugin_data_paths (state_t *state)
{
    char *home_path = getenv ("HOME");

    // breakpoints
    state->break_path = create_path (3, home_path, DATA_DIR_PATH, "breakpoints.lst");

    // watchpoints
    state->watch_path = create_path (3, home_path, DATA_DIR_PATH, "watches.ls");
}



int
get_plugin_code_index (char    *code,
                       state_t *state)
{
    int si = 1,
        mi,
        ei = state->num_plugins - 1, 
        r;
    while (si <= ei) {
        mi = si + (ei - si) / 2;
        r = strcmp (plugin_codes [mi], code);
        if (r == 0) {
            return mi;
        } else if (r < 0) {
            si = mi + 1;
        } else {
            ei = mi - 1;
        }
    }
    return -1;
}



