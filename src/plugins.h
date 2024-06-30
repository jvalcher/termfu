
#ifndef _plugins_h
#define _plugins_h

#include "data.h"



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


void set_num_plugins  (state_t*);

void set_plugin_data_paths (state_t*);

int get_plugin_code_index (char*, state_t*);



#endif
