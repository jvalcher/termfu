
#ifndef _plugins_h
#define _plugins_h

#include "data.h"



/*
    Plugin indexes
    --------
    - Used to identify plugins and access all associated data

        Asm == 0
        plugin_codes[Asm] == "Asm"
        state->plugins[Asm]->...

    - MUST be sorted alphabetically for get_plugin_code_index()  [A-Z,a-z]
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
    Unt,
    Wat
};




/*
    Set number of plugins
    ------
    state->num_plugins
*/
void set_num_plugins  (state_t*);



/*
    Allocate state->plugins[i]
    ---------
    Returns A_OK or FAIL
*/
int allocate_plugins (state_t *state);



/*
    Return plugin code's index
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

    state->plugins[i]->win

    - Returns A_OK, FAIL
*/
int allocate_plugin_windows (state_t*);



/*
    Print plugin indexes, codes
    ---------
    $ make plugins
*/
void print_plugin_indexes_codes (void);



#endif
