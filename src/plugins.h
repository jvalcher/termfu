
#ifndef _plugins_h
#define _plugins_h

#include "data.h"



/*
   Plugin indexes
   --------
   - Order matches respective code strings in *plugin_codes[] in plugins.c  (i.e. alphabetical)
*/
enum {
    Asm,
    AtP,
    Brk,
    Con,
    Dbg,
    Fin,
    Kil,
    Lay,
    LcV,
    Nxt,
    Prg,
    Prm,
    Qut,
    Reg,
    Run,
    Src,
    Stk,
    Stp,
    Trg,
    Unt,
    Wat
};



/*
    Set number of plugins in state->num_plugins
*/
void set_num_plugins  (state_t*);



/*
    Return plugin code index matching its enum above
    ------
    Returns index or FAIL
*/
int get_plugin_code_index (char *code, state_t *state);



/*
    Get plugin code from index
    -------
    Returns pointer to code or NULL
*/
char* get_plugin_code (int plugin_index);



/*
    Allocate plugin window_t structs et al.
    ----------
    Returns A_OK, FAIL
*/
int allocate_plugin_windows (state_t*);



/*
    Print plugin indexes and codes  (debugging)
    ---------
    Run with `make plugins`
*/
void print_plugin_indexes_codes (void);



#endif
