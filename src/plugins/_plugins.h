
#ifndef _plugins_h
#define _plugins_h

#include "../data.h"



enum {
    EMP,
    Asm,
    Bak,
    Bld,
    Brk,
    Con,
    Fin,
    Kil,
    Lay,
    LcV,
    Nxt,
    Out,
    Prm,
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

extern char *plugin_code [];
extern int   num_plugins;
extern char *win_file_name [];



/*
    Plugin function pointers
*/
typedef void (*plugin_func_t) (state_t *state);
    //
extern plugin_func_t plugin_func [];



#endif
