
#ifndef _plugins_h
#define _plugins_h

#include "data.h"



/*
    Plugin indexes
    --------
    - Used to identify plugins and access all associated data

        Asm == 0
        plugin_codes[Asm] == "Asm"
        state->plugins[Asm]->code == "Asm"

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
    Nxi,
    Nxt,
    Prg,
    Prm,
    Qut,
    Reg,
    Run,
    Src,
    Sti,
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
*/
int allocate_plugins (state_t *state);



/*
    Return plugin code's index
*/
int get_plugin_code_index (char *code, state_t *state);



/*
    Get plugin code from index
    -------
    Returns pointer to code in plugin_codes[] or NULL
*/
char* get_plugin_code (int plugin_index);



/*
    Create window_t structs et al; configure
    ----------
    state->plugins[i]->win
    state->plugins[i]->win->topbar_title
    state->plugins[i]->win->buff_data
    state->plugins[i]->win->buff_data->buff
    ...
*/
int allocate_plugin_windows (state_t*);



/*
    Print plugin indexes, codes
    ---------
    $ make plugins
*/
void print_plugin_indexes_codes (void);



#endif
