/////////////////////////////////
//
//  Sections:
//  ---------
//  # Key bindings
//  # Color pair identifiers
//  # Layout configuration data
//  # Debugging
//
/////////////////////////////////


 
#ifndef data_h
#define data_h


#include <ncurses.h>
#include <stdint.h>


/****************
  # Key bindings
 ****************/

/*
    Shortcut keys bound to plugin function indexes
    -------------
    - Declared in render_layout.c and initialized 
      with bind_keys_to_plugins()
    - plugin[i]() called in main() loop  (main.c)  via
      run_plugin()  (run_plugin.c)

        {0,a-z,A-Z}  ->  {0-52}

        {4} == 12  ->  plugin[12]()
        {6} == 0   ->  unassigned

*/
extern int key_function_index [];



/**************************
  # Color pair identifiers
 **************************/

/*
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

#define BORDER_COLOR           BLUE_BLACK
#define MAIN_TITLE_COLOR       GREEN_BLACK
#define LAYOUT_TITLE_COLOR     MAGENTA_BLACK
#define WINDOW_TITLE_COLOR     CYAN_BLACK
#define HEADER_TITLE_COLOR     GREEN_BLACK
#define TITLE_KEY_COLOR        YELLOW_BLACK

#define FOCUS_BORDER_COLOR       YELLOW_BLACK
#define FOCUS_HEADER_TITLE_COLOR TITLE_KEY_COLOR
#define FOCUS_TITLE_KEY_COLOR    HEADER_TITLE_COLOR



/*****************************
  # Layout configuration data
  -------------
    Contains parsed data from external CONFIG_FILE
    Used to render screen and run plugins
    Created in parse_config.c

    layouts_t
        ...
        plugin_t
        window_t

 *****************************/

#define CONFIG_FILE             ".termide"
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    20
#define MAX_KEY_STR_LEN         50
#define MAX_LAYOUTS             10
#define MAX_WINDOWS             10      // per layout
#define MAX_TITLE_LEN           20
#define MAX_SHORTCUTS           52      // a-z, A-Z



/*
    plugin_t
    ---------
    Store plugin, key, title combinations for a single layout

    layouts->plugins[i]

    code       - code string  (Bld, Stp, ...)
    key        - keyboard shortcut character  (b, s, ...)
    title      - title string  ( (s)tep, (r)un, ...)
    next       - next plugin_t struct
*/
typedef struct plugin {

    char           code [4];
    char           key;
    char           title [MAX_TITLE_LEN];
    struct plugin *next;

} plugin_t;



/*
    windows_t
    --------
    - Position, size data for single Ncurses window 
    - All windows in each layout stored in linked list
    
    layouts->windows
  
    win     - Ncurses WINDOW* object
    key     - window segment key
    rows    - height in rows
    cols    - width in columns
    y       - top left corner y coordinate
    x       - top left corner x coordinate
    next    - pointer to next windows_t object in layout
*/  
typedef struct window {

    WINDOW        *win;
    char           key;
    bool           is_focused;
    int            rows;                   
    int            cols;                   
    int            y;                      
    int            x;                      
    struct window *next;           

} window_t;



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
    hdr_key_strs    - array of header plugin key strings  ("ssb\nssw\nccr\n")
    hdr_key_rows    - array of number of header rows needed for key strings
    win_matrices    - array of window key segment* matrices
    row_ratios      - array of y segment ratios for layout matrices
    col_ratios      - array of x segment ratios for layout matrices
    headers         - array of WINDOW headers
    plugins         - array of plugins_t linked lists
    windows         - array of windows_t linked lists
*/
typedef struct layouts {

    int       num;
    int       scr_height;
    int       scr_width;
    char      labels       [MAX_LAYOUTS][MAX_CONFIG_LABEL_LEN];
    char      hdr_key_strs [MAX_LAYOUTS][MAX_KEY_STR_LEN];
    int       hdr_key_rows [MAX_LAYOUTS];
    char     *win_matrices [MAX_LAYOUTS];
    int       row_ratios   [MAX_LAYOUTS];
    int       col_ratios   [MAX_LAYOUTS];
    WINDOW   *headers      [MAX_LAYOUTS];
    plugin_t *plugins      [MAX_LAYOUTS];
    window_t *windows      [MAX_LAYOUTS];

} layouts_t;




/**************
 # Debugging
***************/


//  Number of layouts to print with print_layouts()  (parse_config.c)
//
//      $ make layouts
//
//  TODO: convert to command-line flag
//
#define PRINT_LAYOUTS   1       



#endif
