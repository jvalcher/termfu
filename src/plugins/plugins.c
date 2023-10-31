#include "plugins.h"
#include "../data.h"
  
/*
    Action codes
    -----
    Three-letter, case-sensitive global identifiers for plugins

    Key shortcut and title string associated with each plugin are 
    set in CONFIG_FILE for each layout

    See "Example layout configuration" in `_Readme.md`
*/
char *plugin_codes [] = {

    // misc actions
    "Qut",      // quit termIDE
    "Lay",      // display layouts
    "Bld",      // display builds

    // GDB
    //
        // windows
    "Asm",      // assembly
    "Brk",      // breakpoints
    "Wat",      // watched variables
    "Loc",      // local variables
    "Src",      // source file
    "Reg",      // registers
    "Prt",      // print values
    "Out",      // output
    "Prm",      // command prompt

        // actions
    "Run",      // (re)run program with current build
    "Nxt",      // next
    "Stp",      // step
    "Con",      // continue
    "Fin",      // finish
    "Kil",      // kill program
};



/*
    Print number of plugins with "make num_plugins"
    ----------
    Use to set NUM_PLUGINS in src/data.h
*/
#ifdef NUM_PLUGINS
int main (void)
{
    printf ("\nNumber of plugins: %ld\n\n", sizeof (plugin_codes) / sizeof (plugin_codes [0]));
    return 0;
}
#endif


