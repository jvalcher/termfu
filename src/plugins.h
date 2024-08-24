
#ifndef _plugins_h
#define _plugins_h

#include "data.h"



/*
   Plugin enum identifiers
   --------
   - In alphabetical order
*/
enum {
    EMP,    
    Asm,
    Bak,
    Bld,
    Brk,
    Con,
    Dbg,
    Fin,
    Kil,
    Lay,
    LcV,
    Nxt,
    Prg,
    Qut,
    Reg,
    Run,
    ScD,
    ScL,
    ScR,
    ScU,
    Src,
    Stp,
    Unt,
    Wat
};



/*
   Get plugin code from its index
*/
char* get_plugin_code (int plugin_index);



/*
    Calculate and set number of plugins
    ---------
    Used in parse_config_file()
*/
void set_num_plugins  (state_t*);



/*
    Return plugin code index matching its enum above
    ------
    Used in parse_config_file()
*/
int get_plugin_code_index (char*, state_t*);



/*
    Label plugins with and without windows
    --------
    Configure buffer or file data source
*/
void set_window_plugins (state_t*);



#endif
