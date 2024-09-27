 
#ifndef DATA_H
#define DATA_H

#include <ncurses.h>
#include <time.h>



#define PROGRAM_NAME          "termfu"
#define CONFIG_FILE           ".termfu"             // default (see -c flag)
#define PERSIST_FILE          ".termfu_data"        // default (see -p flag)
#define CONFIG_COMMAND_LABEL  "command"             // CONFIG_FILE section header
#define CONFIG_PLUGINS_LABEL  "plugins"             //   "
#define CONFIG_LAYOUTS_LABEL  "layout"              //   "



/*******
  Debug
 *******/

#define A_OK   0
#define FAIL  -1

#define ERR_DBG_CMD     "Failed to send debugger command"
#define ERR_DBG_PARSE   "Failed to parse debugger output"
#define ERR_DISP_LINES  "Failed to display lines"
#define ERR_NC_ATTR     "Failed to set ncurses attribute"
#define ERR_OUT_MARK    "Failed to insert output marker"
#define ERR_POPUP_IN    "Failed to get popup window input"
#define ERR_PULSE_CMD   "Failed to pulse header title color"
#define ERR_REND_LAYOUT "Failed to render layout"
#define ERR_UPDATE_WIN  "Failed to update window"
#define ERR_UPDATE_WINS "Failed to update windows"

#define DEBUG_OUT_FILE  "debug.out"           // logd() output file
#define DEBUG_PID_FILE  "/tmp/termfu.pid"     // used by `make proc_<debugger>`



/*********
  Ncurses
 *********/

#define ESC  27

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

// misc
#define DEFAULT                         WHITE_BLACK
#define MAIN_TITLE_COLOR                GREEN_BLACK
#define LAYOUT_TITLE_COLOR              MAGENTA_BLACK
#define TITLE_KEY_COLOR                 YELLOW_BLACK

// header
#define HEADER_TITLE_COLOR              GREEN_BLACK
#define FOCUS_TITLE_KEY_COLOR           GREEN_BLACK
#define FOCUS_HEADER_TITLE_COLOR        YELLOW_BLACK

// window
#define WINDOW_TITLE_COLOR              CYAN_BLACK
#define BORDER_COLOR                    BLUE_BLACK
#define FOCUS_WINDOW_TITLE_COLOR        YELLOW_BLACK
#define FOCUS_BORDER_COLOR              YELLOW_BLACK
#define FOCUS_WINDOW_TITLE_KEY_COLOR    BLUE_BLACK
#define WINDOW_INPUT_TITLE_COLOR        WHITE_BLUE

// form
#define FORM_INPUT_FIELD                WHITE_BLUE
#define FORM_BUTTON                     WHITE_BLUE



/*********
  Layouts
 *********/

#define FIRST_LAYOUT         "FiRsT_lAyOuT"
#define LAYOUT_CATEG_LEN     128
#define LAYOUT_LABEL_LEN     128
#define LAYOUT_KEY_STR_LEN   256
#define LAYOUT_TITLE_LEN     48
#define LAYOUT_ROW_SEGMENTS  24      // per layout
#define LAYOUT_COL_SEGMENTS  24

/*
    layout_t
    -------
    state->layouts

    label               - layout title, displayed at top of screen
    hdr_key_str         - header layout string derived from CONFIG_FILE
    win_key_str         - window layout string derived from CONFIG_FILE
    num_hdr_key_rows    - number of lines of commands in header
    win_matrix          - window key matrix, used to render window layout
    row_ratio           - number of rows of keys in CONFIG_FILE ASCII-art window layout
    col_ratio           - number of cols of keys in CONFIG_FILE ASCII-art window layout
    next                - next layout_t in linked list
*/
typedef struct layout {

    char            label       [LAYOUT_LABEL_LEN];
    char            hdr_key_str [LAYOUT_KEY_STR_LEN];
    char            win_key_str [LAYOUT_KEY_STR_LEN];
    int             num_hdr_key_rows;
    char          **win_matrix;
    int             row_ratio;
    int             col_ratio;
    struct layout  *next;

} layout_t;



/*********
  Ncurses window data
 *********/

#define CODE_LEN          3
#define NO_DATA_MSG       "Not supported by "
#define FILE_PATH_LEN     256
#define ADDRESS_LEN       48
#define FUNC_LEN          128
#define ORIG_BUF_LEN      16384     // Double buffer size 3x to 32768 -> 65536 -> 131072
#define MAX_DOUBLE_TIMES  3         // before it loops back around                            
                                        
/*
    buff_data_t
    --------
    state->plugins[i]->win->buff_data

    buff        - window data buffer
    buff_len    - buffer size
    buff_pos    - current index position of terminating null
    changed     - signals for buffer data to be reloaded before being displayed
    rows        - lines of data in buffer 
    max_cols    - last index of longest line
    scroll_row  - current scroll row
    scroll_col  - first character of each line to be displayed
    new_data    - signals that new Dbg or Prg window data is in the debugger's buffer(s) 
                  and should be appended to the respective window_t buffer
*/
typedef struct {

    char   code [CODE_LEN+1];
    char  *buff;  
    int    buff_len;
    int    buff_pos;
    int    times_doubled;
    bool   changed;
    int    rows;
    int    max_cols;
    int    scroll_row;
    int    scroll_col;
    bool   new_data;

} buff_data_t;

/*
    src_file_data_t
    --------
    state->plugins[i]->win->src_file_data

    ptr                 - source file pointer
    line                - current display line
    line_num_digits     - number of columns required to display line numbers
    first_char          - column index of first character to be displayed for each line
    rows                - number of lines in file
    max_cols            - column index of last character in longest line
    min_mid             - minimum middle line to display given number of rows in window
    max_mid             - maximum middle line to display given number of rows in window
    offsets             - first character of each line offsets

    path                - current file path buffer
    path_len            - buffer size
    path_pos            - current index of null terminator
    path_changed        - signals that there is a new source file that must be processed before display
    addr                - current hex address of program's execution buffer
    addr_len            - buffer size
    addr_pos            - current index of null terminator
    func                - current function buffer
    func_len            - buffer size
    func_pos            - current index of null terminator
*/
typedef struct {

    FILE  *ptr;
    int    line;
    int    line_num_digits;
    int    first_char;
    int    rows;
    int    max_cols;
    int    min_mid;
    int    max_mid;
    long  *offsets;

    char   path [FILE_PATH_LEN];    
    int    path_len;
    int    path_pos;
    bool   path_changed;
    char   addr [ADDRESS_LEN];
    int    addr_len;
    int    addr_pos;
    char   func [FUNC_LEN];
    int    func_len;
    int    func_pos;

} src_file_data_t;

/*
    window_t
    ------
    - Individual ncurses WINDOW data
    state->plugins[i]->win

    WIN                 - parent ncurses WINDOW
    TWIN                - topbar ncurses subWINDOW
    DWIN                - data ncurses subWINDOW

    key                 - plugin key binding
    code                - plugin code
    has_topbar          - signals window has topbar

    rows                - parent rows
    cols                - columns
    y                   - top left screen row coordinate
    x                   - top left screen column coordinate
    border              - border settings required by wborder()

    topbar_rows         - topbar subwindow rows
    topbar_cols         - columns
    topbar_y            - top left row coordinate relative to parent window
    topbar_x            - top left column coordinate    "
    topbar_title        - title string
                        
    data_win_rows       - data subwindow rows
    data_win_cols       - columns
    data_win_y          - top left row coordinate relative to parent window
    data_win_x          - top left column coordinate     "
    data_win_mid_line   - middle row index

    buff_data           - buff_data_t pointer
    src_file_data       - src_file_data_t pointer
*/
typedef struct {

    WINDOW       *WIN;
    WINDOW       *TWIN;
    WINDOW       *DWIN;

    int           key;
    char          code[CODE_LEN + 1];
    bool          has_topbar;

    // parent window
    int           rows;                   
    int           cols;                   
    int           y;                      
    int           x;                      
    int           border [8];

    // topbar title subwindow
    int           topbar_rows;
    int           topbar_cols;
    int           topbar_y;
    int           topbar_x;
    char         *topbar_title;

    // data subwindow
    int           data_win_rows;
    int           data_win_cols;
    int           data_win_y;
    int           data_win_x;
    int           data_win_mid_line;

    // buffer, source file window data
    buff_data_t      *buff_data;
    src_file_data_t  *src_file_data;

} window_t;



/**********
  Debugger
 **********/

#define PIPE_READ        0
#define PIPE_WRITE       1
#define DEBUG_TITLE_LEN  8
#define PROGRAM_PATH_LEN 256
#define READER_BUF_LEN   8192

enum { DEBUGGER_GDB, DEBUGGER_PDB };
enum { READER_RECEIVING, READER_DONE };

/*
    debugger_t
    -------
    state->debugger

    index                   - DEBUGGER_GDB, DEBUGGER_PDB, ...
    title                   - "gdb", "pdb", ...
    running                 - controls main() loop state
    prog_path               - binary, main script path buffer
    prog_update_time        - file's last update time (st_mtim.tv_sec), used to signal reload
                                        
    stdin_pipe;             - debugger process input pipe
    stdout_pipe;            - debugger process output pipe
                                        
    reader_state            - READER_RECEIVING, RECEIVER_DONE
    reader_buffer           - debugger process initial output buffer
                                        
    format_buffer           - misc buffer for formatting output
    format_len              - buffer size
    format_pos              - null terminator index
    format_times_doubled    - times buffer size doubled
    data_buffer             - data stream buffer
    data_len                - buffer size
    data_pos                - null terminator index
    data_times_doubled      - times buffer size doubled
    cli_buffer              - cli stream buffer
    cli_len                 - buffer size
    cli_pos                 - null terminator index
    cli_times_doubled       - times buffer size doubled
    program_buffer          - program output buffer
    program_len             - buffer size
    program_pos             - null terminator index
    program_times_doubled   - times buffer size doubled
    async_buffer            - async (status) buffer
    async_len               - buffer size
    async_pos               - null terminator index
    async_times_doubled     - times buffer size doubled
*/
typedef struct {

    int     index;
    char    title         [DEBUG_TITLE_LEN];
    bool    running;
    char    prog_path     [PROGRAM_PATH_LEN];
    time_t  prog_update_time;

    int     stdin_pipe;
    int     stdout_pipe;

    int     reader_state;
    char    reader_buffer  [READER_BUF_LEN];

    char   *format_buffer;
    int     format_len;
    int     format_pos;
    int     format_times_doubled;
    char   *data_buffer;
    int     data_len;
    int     data_pos;
    int     data_times_doubled;
    char   *cli_buffer;
    int     cli_len;
    int     cli_pos;
    int     cli_times_doubled;
    char   *program_buffer;
    int     program_len;
    int     program_pos;
    int     program_times_doubled;
    char   *async_buffer;
    int     async_len;
    int     async_pos;
    int     async_times_doubled;

} debugger_t;



/*********
  Plugins
 *********/

#define PLUGIN_CODE_LEN  3

#define BEG_DATA         0      // display beginning buffer, file data
#define END_DATA         1      // display end of buffer, file data
#define LINE_DATA        2      // center data on state->plugins[i]->win->src_file_data->line
#define ROW_DATA         3      // center data on state->plugins[i]->win->buff_data->scroll_row

#define BUFF_TYPE        0      // buff_data_t
#define FILE_TYPE        1      // src_file_data_t

/*
    plugin_t
    ------
    - Indexes matche those set in enum array in plugins.h
    state->plugins[i]

    key         - plugin key binding
    code        - plugin code string
    title       - plugin header/window title string
    data_pos    - BEG_DATA, END_DATA, LINE_DATA
    win_type    - BUFF_TYPE, FILE_TYPE
    has_window  - plugin has window in current layout
*/
typedef struct {

    char      key;
    char      code [PLUGIN_CODE_LEN + 1];
    char     *title;
    int       data_pos;         // BEG_DATA, END_DATA, LINE_DATA
    int       win_type;         // BUFF_TYPE, FILE_TYPE
    bool      has_window;       // in current layout
    window_t *win;

} plugin_t;



/*******
  State
 *******/

#define CONFIG_PATH_LEN  128
#define PID_LEN          8
#define DATA_PATH_LEN    CONFIG_PATH_LEN
#define BREAK_LEN        128
#define WATCH_LEN        84
#define INPUT_BUFF_LEN   4096

/*
    breakpoint_t
    -------
    state->breakpoints

    index       - breakpoint index set by debugger
    path_line   - <file path>:<line_number> string
    next        - next breakpoint_t in linked list
*/
typedef struct breakpoint {

    char  index [8];
    char  path_line [BREAK_LEN];
    struct breakpoint *next;

} breakpoint_t;

/*
    watchpoint_t
    -------
    state->wathcpoints

    index   - custom index
    var     - variable buffer
    value   - value buffer
    next    - next watchpoint_t in linked list
*/
typedef struct watchpoint {

    int   index;
    char  var   [WATCH_LEN];
    char  value [WATCH_LEN];
    struct watchpoint *next;

} watchpoint_t;

/*
    state_t
    -----
    - Base data struct that contains all the above structs
    - state->...

    num_plugins         - total number of plugins
    plugin_key_index    - array that matches key binding to its plugin's index  (see plugins.h)
                          - plugin_key_index [key] = plugin index
    config_path         - configuration file path set by -c flag
    data_path           - data persistence path set by -p flag
    input_buffer        - popup window input buffer

    plugins             - pointer array for plugin_t structs

    layouts             - linked list of layout_t structs
    curr_layout         - current layout_t struct
    header              - ncurses header WINDOW

    debugger            - debugger_t struct
    command             - debugger command string array for execvp()
    wathcpoints         - linked list of watchpoint_t structs
    breakpoints         - linked list of breakpoint_t structs
*/
typedef struct {

    int            num_plugins;
    int           *plugin_key_index;
    char           config_path  [CONFIG_PATH_LEN];
    char           data_path    [DATA_PATH_LEN];
    char           input_buffer [INPUT_BUFF_LEN];

    plugin_t     **plugins;

    layout_t      *layouts;
    layout_t      *curr_layout;
    WINDOW        *header;

    debugger_t    *debugger;
    char         **command;
    watchpoint_t  *watchpoints;
    breakpoint_t  *breakpoints;

} state_t;



#endif

