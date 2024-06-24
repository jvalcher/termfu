 
#ifndef _data_h
#define _data_h


#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>



/***********
   termIDE
 ***********/

#define RENDER_WINDOW_SEM_NAME  "/render_window_sem"
#define PARENT_PROCESS  0
#define CHILD_PROCESS   1
extern char *prog_name;




/*********************
   termIDE debugging
 *********************/

#ifdef LAYOUT_DEBUG
#define PRINT_LAYOUTS  1
#endif



/**************************
   Color pair identifiers
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

// shared
#define MAIN_TITLE_COLOR       GREEN_BLACK
#define LAYOUT_TITLE_COLOR     MAGENTA_BLACK
#define TITLE_KEY_COLOR        YELLOW_BLACK

// header
#define HEADER_TITLE_COLOR       GREEN_BLACK
#define FOCUS_TITLE_KEY_COLOR    GREEN_BLACK
#define FOCUS_HEADER_TITLE_COLOR YELLOW_BLACK

// window
#define WINDOW_TITLE_COLOR              CYAN_BLACK
#define BORDER_COLOR                    BLUE_BLACK
#define FOCUS_WINDOW_TITLE_COLOR        YELLOW_BLACK
#define FOCUS_BORDER_COLOR              YELLOW_BLACK
#define FOCUS_WINDOW_TITLE_KEY_COLOR    BLUE_BLACK



/*****************************
   Layout configuration data
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
#define DATA_DIR_PATH           ".local/share/termide"
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    20
#define MAX_KEY_STR_LEN         50
#define MAX_LAYOUTS             10
#define MAX_WINDOWS             10      // per layout
#define MAX_TITLE_LEN           20
#define MAX_ROW_SEGMENTS        10      // per layout
#define MAX_COL_SEGMENTS        10
#define MAX_SHORTCUTS           52      // a-z, A-Z



/***********
   structs
 ***********/


/*
    window_t
    --------
    - All windows for single layout
    - Ncurses window object, key, status
    - Position, size data for Ncurses window 
    - Scrolling data
    - Stored in linked list

        state->layout->windows

    win             - Ncurses WINDOW* object
    key             - window segment key
    selected        - is current window selected
    next            - pointer to next window_t object in layout

    win_rows        - height in rows
    win_cols        - width in columns
    win_y           - top left corner y coordinate
    win_x           - top left corner x coordinate
    win_mid_line    - middle row of window

	file_path       - current src/data file's absolute path
    file_ptr        - FILE pointer
    file_first_char - first character (col) displayed of lines
    file_rows       - number of file rows
    file_max_cols   - columns in longest file line
    file_min_mid    - mininimum mid line (beginning of file)
    file_max_mid    - maximum mid line (end of file)
    file_offsets    - byte offsets for each line of file
*/  
typedef struct window {

    WINDOW            *win;
    char               key;
    bool               selected;
    struct window     *next;           

    int                win_rows;                   
    int                win_cols;                   
    int                win_y;                      
    int                win_x;                      
    int                win_border [8];
    int                win_mid_line;

    char              *out_file_path;
    int                file_first_char;
    int                file_rows;
    int                file_max_cols;
    int                file_min_mid;
    int                file_max_mid;
    unsigned long int *file_offsets;

} window_t;



/*
    layout_t
    ---------
    - Main layout configuration struct for: 
        - rendering layouts
    - Stored in linked list
    - Created in parse_config.c from external CONFIG_FILE

        state->layouts
  
    label             - layout label string
    header            - Ncurses WINDOW object for header
    hdr_key_str       - header plugin key string  ("ssb\nssw\nccr\n")
    win_key_str       - window plugin key string  ("ssb\nssw\nccr\n")
    num_hdr_key_rows  - number of header rows needed for key strings
    win_matrix        - window key segment* matrix
    row_ratio         - y segment ratio for layout matrix
    col_ratio         - x segment ratio for layout matrix
    windows           - window_t linked list
    next              - next layout_t struct   (TODO: remove?, re-render for new layout?)
*/
typedef struct layout {

    char           label [MAX_CONFIG_LABEL_LEN];
    WINDOW        *header;
    char           hdr_key_str [MAX_KEY_STR_LEN];
    char           win_key_str [MAX_KEY_STR_LEN];
    int            num_hdr_key_rows;
    char         **win_matrix;
    int            row_ratio;
    int            col_ratio;
    window_t      *windows;
    struct layout *next;

} layout_t;



/*
    plugin_t
    ---------
    - All plugins' info
    - Stored in linked list

        state->plugins

    code        - code string  (Bld, Stp, ...)
    key         - keyboard shortcut character  (b, s, ...)
    title       - title string  ( (s)tep, (r)un, ...)
    window      - pointer to window_t struct if applicable
    next        - next plugin_t struct
*/
typedef struct plugin {

    char           code [4];
    char           key;
    char           title [MAX_TITLE_LEN];
    window_t*      window;
    struct plugin *next;

} plugin_t;



/*
    debug_state_t
    ------
    - Current debugger state

        state->debug_state

    debugger        - debugger enum identifier
    input_pipe      - debugger process input pipe
    output_pipe     - debugger process output pipe
    prog_path       - program path
    out_file_path   - output file path
    break_point     - breakpoint string to be set     
    out_done_str    - string that indicates output is finished
    exit_str        - string that indicates debugger process exited
*/

// Debugger
enum { DEBUGGER_UNKNOWN, DEBUGGER_GDB };

// Debugger reader process state
enum { READER_RECEIVING, READER_DONE, READER_EXIT };

typedef struct debug_state {

    int     debugger;
    char  **cmd;
    pid_t   debugger_pid;
    int     input_pipe;
    int     output_pipe;

    char   *prog_path;
    char   *prg_out_path;
    char   *dbg_out_path;

} debug_state_t;



/*
   win_cmds_t
   --------
   - Keys that can be or are exclusive to window use
   - Set in bind_keys_windows_to_plugins()
*/
typedef struct win_keys {

    int back;
    int quit;
    int scroll_down;
    int scroll_up;
    int scroll_left;
    int scroll_right;

} win_keys_t;



/*
   state_t
   -----
   - Collection of all states

   curr_layout    - Current layout
   layouts        - Linked list of layout_t structs
   plugins        - Linked list of plugin_t structs
   debug_state    - Current debugger state
*/
typedef struct state {

    bool            running;
    int             process;

    layout_t       *curr_layout;
    window_t       *curr_window;
    plugin_t       *curr_plugin;

    layout_t       *layouts;
    plugin_t       *plugins;
    debug_state_t  *debug_state;
    win_keys_t     *win_keys;

} state_t;


#endif
