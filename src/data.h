 
#ifndef _DATA_H
#define _DATA_H


#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>



/******
  Main
 ******/

#define PROGRAM_NAME     "termIDE"
#define DATA_DIR_PATH    ".local/share/termide"
#define PARENT_PROCESS   0
#define CHILD_PROCESS    1
#define PLUGIN_CODE_LEN  3



/*********
  Ncurses
 *********/

/*
   Color pair identifiers

        <TEXT>_<BACKGROUND>
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
#define WINDOW_INPUT_COLOR              WHITE_BLUE



/*********
  Layouts
 *********/

#define CONFIG_FILE             ".termide"
#define FIRST_LAYOUT            "FiRsT_lAyOuT"
#define MAX_CONFIG_CATEG_LEN    20
#define MAX_CONFIG_LABEL_LEN    20
#define MAX_KEY_STR_LEN         50
#define MAX_LAYOUTS             10
#define MAX_WINDOWS             10      // per layout
#define MAX_TITLE_LEN           20
#define MAX_ROW_SEGMENTS        10      // per layout
#define MAX_COL_SEGMENTS        10
#define MAX_SHORTCUTS           52      // a-z, A-Z

/*
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

    char           label [MAX_CONFIG_LABEL_LEN];        // TODO: allocate
    char           hdr_key_str [MAX_KEY_STR_LEN];
    char           win_key_str [MAX_KEY_STR_LEN];
    int            num_hdr_key_rows;
    char         **win_matrix;
    int            row_ratio;
    int            col_ratio;
    struct layout *next;

} layout_t;



/*********
  Windows
 *********/

/*
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
#define MAX_LINES  128

typedef struct {

    WINDOW  *WIN;
    WINDOW  *IWIN;
    WINDOW  *DWIN;
    bool     selected;

    bool     has_input;
    char    *input_inactive_str;
    char    *input_active_str;

    int      rows;                   
    int      cols;                   
    int      y;                      
    int      x;                      
    int      border [8];

    int      scroll_y;
    int      scroll_x;

    int      buff_max_cols;
    int      buff_rows;

} window_t;

typedef struct {

    bool      path_changed;
    char     *path;
    int       rows;
    int       max_cols;
    int       min_mid;
    int       max_mid;
    int      *offsets;
    window_t *win;

} src_file_t;



/**********
  Debugger
 **********/

#define PIPE_READ            0
#define PIPE_WRITE           1
#define READER_MSG_MAX_LEN  24
#define CMD_MAX_LEN         256
#define DEBUG_BUF_LEN       4096

enum { DEBUGGER_GDB };
enum { READER_RECEIVING, READER_DONE, READER_EXIT };

typedef struct {

    int           curr;
    bool          running;
    char        **cmd;
    char         *prog_path;

    int           stdin_pipe;
    int           stdout_pipe;
    int           stderr_pipe;      // TODO: not used

    char          debugger_buffer [DEBUG_BUF_LEN],
                  program_buffer  [DEBUG_BUF_LEN];


} debugger_t;

typedef struct {

    int   state;
    char  output_line_buffer [1024];
    char *debugger_buffer_ptr;
    char *program_buffer_ptr;
    int   plugin_index;

} reader_t;



/*********
  Plugins
 *********/

/*
    code        - code string  (Bld, Stp, ...)
    key         - keyboard shortcut character  (b, s, ...)
    title       - title string  ( (s)tep, (r)un, ...)
    window      - pointer to window_t struct if applicable
    next        - next plugin_t struct
*/
typedef struct {

    int       index;
    char      key;
    char      code [4];
    char     *title;       // TODO: allocate
    bool      has_window;
    window_t *win;

} plugin_t;



/*******
  State
 *******/

#define PATH_MAX_LEN  256

typedef struct state_t state_t;
typedef void (*send_cmd_t)   (int, state_t*);
typedef void (*update_win_t) (int, state_t*);

typedef struct state_t {

    int           num_plugins;
    int          *plugin_key_index;
    char         *break_path;
    char         *watch_path;

    layout_t     *layouts;
    window_t    **windows;
    src_file_t   *src_file;
    plugin_t    **plugins;
    debugger_t   *debugger;

    WINDOW       *header;

} state_t;



#endif
