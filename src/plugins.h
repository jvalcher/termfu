
#ifndef _plugins_h
#define _plugins_h

#include "data.h"



/*
   Plugin index identifiers
   --------
   - In alphabetical order
*/
enum {
    EMP,    
    Asm,
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
    Allocate plugin window_t structs et al.
*/
void allocate_plugin_windows (state_t*);



#endif
