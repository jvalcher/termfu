
/*
    Parse configuration file data

    print_layouts ()        - print contents of layouts_t struct (parse_config.h)
*/

#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H


#include "data.h"


/*
   Parse external configuration file CONFIG_FILE located
   in current or home directory
   ---------
  
   CONFIG_FILE format:
  
       [<category>:<label>]
       <value>
  
   Categories:
  
       layout   - new window layout scheme
       ...
  
   Example CONFIG_FILE settings:
  
        [layout:src_asm_1]
        sab
        saw
        sar
*/  
layout_t* parse_config (void);



/*
    Print data for first <n> layouts, pause between each
    ---------
    For debugging

    Set n with: 

        src/data.h : PRINT_LAYOUTS

    Call with:

        $ make layouts

*/
void print_layouts (int n, 
                    layout_t* layout);



#endif
