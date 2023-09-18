/*
    Data structures for layouts and the windows
    ---------
    Each layout consists of multiple windows defined in 
    external configuration file (see config.h)
*/

#ifndef PARSE_CONFIG_LAYOUT_H
#define PARSE_CONFIG_LAYOUT_H

#include "get_config.h"


#define MAX_WINDOW_TYPES    26
#define MAX_LAYOUTS         10
#define MAX_TITLE_LEN       20
#define MAX_WINDOWS         20      // per layout
#define MAX_Y_SEGMENTS      10      // per layout
#define MAX_X_SEGMENTS      10      // per layout


//  window
//  ------
//  Holds data for single window
//  Stored in layouts struct below
//  Each layout's windows stored as linked list
//
//  symbol  - window symbol character (see config.h)
//  title   - title string
//  rows    - height in rows
//  cols    - width in columns
//  y       - top left corner y coordinate
//  x       - top left corner x coordinate
//  next    - pointer to next window
//
struct window {
    char   symbol;                 
    char   title [MAX_TITLE_LEN];  
    int    rows;                   
    int    cols;                   
    int    y;                      
    int    x;                      
    struct window* next;           
};


//  layouts
//  -------
//  Contains all layouts, each composed of one or more windows
//
//  num_layouts - number of layouts
//  num_windows - number of windows in each layout
//  labels      - array of label strings  ( [layout:<label>], see config.h)
//  windows     - array of window struct pointers, each pointing to head of window linked list
//
struct layouts {
    int    num_layouts;                      
    int    num_windows     [MAX_LAYOUTS];    
    char   labels          [MAX_TITLE_LEN];  
    struct window *windows [MAX_WINDOWS];    
};


// parse_config_layouts ()
// ----------
// Parse config layouts from config_file_data
// Store data in layouts struct
//
void apply_config_layout (
        int i,
        int scr_rows,
        int scr_cols,
        struct config_file_data *data, 
        struct layouts *layouts );

#endif
