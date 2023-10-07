
/*
    Parse configuration file data
    -----------------------------
    print_layouts ()        - print contents of layouts_t struct (parse_config.h)


*/

#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H



#define NUM_ACTIONS             7

#define CONFIG_FILE             ".termide"
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    20
#define MAX_CONFIG_VALUE_LEN    100

#define MAX_LAYOUTS     10
#define MAX_WINDOWS     10      // per layout
#define MAX_LABEL_LEN   20


// *action_codes []
// ------
// Two-character codes representing actions defined in actions.c to which
// shortcut keys set in CONFIG_FILE (parse_config.h) are assigned by 
// parse_config() below
//
//   CONFIG_FILE:
//
//      <code> : <key> : <window title>
//
//      wa : w : (w)atch
//
extern char *action_codes [];
extern char *window_names [];
extern char  action_keys [];

//  windows_t
//  --------
//  Position, size data for single window 
//
//  win     - ncurses WINDOW* object
//  key     - window segment key
//  rows    - height in rows
//  cols    - width in columns
//  y       - top left corner y coordinate
//  x       - top left corner x coordinate
//  next    - pointer to next windows_t object in layout
//
typedef struct windows {

    WINDOW         *win;
    char            key;
    int             rows;                   
    int             cols;                   
    int             y;                      
    int             x;                      
    struct windows *next;           

} windows_t;


//  layouts_t
//  ---------
//  Window rendering data
//  Calculated in parse_config.c and used in render_screen.c
//
//  screen_change   - screen dimensions have changed
//  num             - number of layouts
//  codes           - action code  (actions.h)
//  labels          - array of layout label strings
//  matrices        - array of layout window segment* matrices
//  y_ratios        - array of y segment ratios for layout matrices
//  x_ratios        - array of x segment ratios for layout matrices
//  windows         - array of window_t linked lists  (render_screen.h)
//
//  *The actual dimensions of the segments and positions of the 
//  respective windows are calculated in render_screen.c
//
typedef struct {

    // TODO: screen_change
    int        num;
    int        scr_height;
    int        scr_width;
    char       codes      [MAX_LAYOUTS][3];
    char       labels     [MAX_LAYOUTS][MAX_LABEL_LEN];
    char      *matrices   [MAX_LAYOUTS];
    int        row_ratios [MAX_LAYOUTS];
    int        col_ratios [MAX_LAYOUTS];
    windows_t *windows    [MAX_LAYOUTS];

} layouts_t;


// Parse external configuration file CONFIG_FILE located
// in current or home directory
// ---------
//
// CONFIG_FILE format:
//
//     [<category>:<label>]
//     <value>
//
// Categories:
//
//     layout   - new window layout scheme
//     ...
//
// Example CONFIG_FILE settings:
//
//      [layout:src_asm_1]
//      sab
//      saw
//      sar
//  
void parse_config (layouts_t*);


// Print data for first <n> layouts
void print_layouts (int n, layouts_t *layouts);



#endif
