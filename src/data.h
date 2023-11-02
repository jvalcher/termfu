/////////////////////////////////
//
//  Sections:
//  ---------
//  # Debugging
//  # Color pair identifiers
//  # Layout configuration data
//
/////////////////////////////////


 
#ifndef data_h
#define data_h


#include <ncurses.h>



/*
    # Debugging
*/


//  Number of layouts to print with: 
//
//      src/parse_config.c : print_layouts()
//
//  Usage:
//
//      $ make layouts
//
#define PRINT_LAYOUTS   1       



/////////////////////////////////


/* 
    # Color pair identifiers
    ----------------------
    Created in src/main.c
    
        <font>_<background>
*/
#define RED_BLACK       20
#define GREEN_BLACK     21
#define YELLOW_BLACK    22
#define BLUE_BLACK      23
#define MAGENTA_BLACK   24
#define CYAN_BLACK      25
#define WHITE_BLACK     26
#define WHITE_BLUE      27

#define BORDER_COLOR        BLUE_BLACK
#define MAIN_TITLE_COLOR    GREEN_BLACK
#define LAYOUT_TITLE_COLOR  MAGENTA_BLACK
#define HEADER_TITLE_COLOR  GREEN_BLACK
#define TITLE_KEY_COLOR     YELLOW_BLACK
#define WINDOW_TITLE_COLOR  CYAN_BLACK



/////////////////////////////////


/*
    # Layout configuration data
    -------------
    Contains parsed data from external CONFIG_FILE
    Used to render screen and run plugins
    Created in parse_config.c

    layouts_t
        ...
        plugins_t
        windows_t
*/
#define CONFIG_FILE             ".termide"
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    20
#define MAX_KEY_STR_LEN         50
#define MAX_LAYOUTS             10
#define MAX_WINDOWS             10      // per layout
#define MAX_TITLE_LEN           20
#define NUM_PLUGINS             18      // get value from "make num_plugins"


/*
    plugins_t
    ---------
    Store plugin, key, title combinations for a single layout

    num:        number of plugins
    codes:      plugin codes 
    keys:       keyboard shortcuts
    key_rows:   number of header rows
    titles:     plugin title string
*/
typedef struct plugins {

    int  num;
    char codes    [NUM_PLUGINS][4];
    char keys     [NUM_PLUGINS][1];
    char titles   [NUM_PLUGINS][MAX_TITLE_LEN];

} plugins_t;


/*
    windows_t
    --------
    Position, size data for single Ncurses window 
    Stored as linked list in layouts_t->windows
  
    win     - ncurses WINDOW* object
    key     - window segment key
    rows    - height in rows
    cols    - width in columns
    y       - top left corner y coordinate
    x       - top left corner x coordinate
    next    - pointer to next windows_t object in layout
*/  
typedef struct windows {

    WINDOW         *win;
    char            key;
    bool            is_focused;
    int             rows;                   
    int             cols;                   
    int             y;                      
    int             x;                      
    struct windows *next;           

} windows_t;


/*
    layouts_t
    ---------
    Main layout configuration struct for: 
        - rendering layouts
        - binding keys to plugin codes

    Created in parse_config.c from external CONFIG_FILE
  
    num             - number of layouts
    scr_height      - screen height
    scr_width       - screen width
    labels          - array of layout label strings
    hdr_key_strs    - array of header plugin key strings
    hdr_key_rows    - array of number of header rows needed for key strings
    win_matrices    - array of window key segment* matrices
    row_ratios      - array of y segment ratios for layout matrices
    col_ratios      - array of x segment ratios for layout matrices
    plugins         - array of plugins_t
    windows         - array of windows_t linked lists
*/
typedef struct layouts {

    int        num;
    int        scr_height;
    int        scr_width;
    char       labels       [MAX_LAYOUTS][MAX_CONFIG_LABEL_LEN];
    char       hdr_key_strs [MAX_LAYOUTS][MAX_KEY_STR_LEN];
    int        hdr_key_rows [MAX_LAYOUTS];
    char      *win_matrices [MAX_LAYOUTS];
    int        row_ratios   [MAX_LAYOUTS];
    int        col_ratios   [MAX_LAYOUTS];
    plugins_t *plugins      [MAX_LAYOUTS];
    windows_t *windows      [MAX_LAYOUTS];

} layouts_t;



/////////////////////////////////




#endif
