 
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
#define ERR_DBG_FCLOSE  "Failed to close DEBUG_OUT_FILE"
#define ERR_DBG_PARSE   "Failed to parse debugger output"
#define ERR_DISP_LINES  "Failed to display lines"
#define ERR_NC_ATTR     "Failed to set ncurses attribute"
#define ERR_NC_FREE     "Failed to free ncurses layout windows"
#define ERR_OUT_MARK    "Failed to insert output marker"
#define ERR_PERSIST     "Failed to persist data"
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
#define RED_BLACK           20
#define GREEN_BLACK         21
#define YELLOW_BLACK        22
#define BLUE_BLACK          23
#define MAGENTA_BLACK       24
#define CYAN_BLACK          25
#define WHITE_BLACK         26
#define WHITE_BLUE          27
#define BLACK_BLUE          28

// misc
#define DEFAULT                       WHITE_BLACK
#define TITLE_KEY_COLOR               YELLOW_BLACK

// header
#define MAIN_TITLE_COLOR              GREEN_BLACK
#define LAYOUT_TITLE_COLOR            MAGENTA_BLACK
#define HEADER_TITLE_COLOR            GREEN_BLACK
#define FOCUS_TITLE_KEY_COLOR         GREEN_BLACK
#define FOCUS_HEADER_TITLE_COLOR      YELLOW_BLACK

// window
#define WINDOW_TITLE_COLOR            CYAN_BLACK
#define BORDER_COLOR                  BLUE_BLACK
#define FOCUS_WINDOW_TITLE_COLOR      YELLOW_BLACK
#define FOCUS_BORDER_COLOR            YELLOW_BLACK
#define FOCUS_WINDOW_TITLE_KEY_COLOR  BLUE_BLACK
#define TOPBAR_COLOR                  WHITE_BLUE
#define BREAK_INDEX_COLOR             YELLOW_BLACK
#define BREAK_FILE_COLOR              GREEN_BLACK
#define BREAK_LINE_COLOR              MAGENTA_BLACK
#define LOC_VAR_COLOR                 GREEN_BLACK
#define PROG_OUT_NEW_RUN_COLOR        GREEN_BLACK
#define SRC_LINE_COLOR                GREEN_BLACK
#define SRC_BREAK_LINE_COLOR          MAGENTA_BLACK
#define WAT_INDEX_COLOR               YELLOW_BLACK
#define WAT_VAR_COLOR                 GREEN_BLACK

// form
#define FORM_INPUT_FIELD              WHITE_BLUE
#define FORM_BUTTON                   WHITE_BLUE



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
    hdr_key_str         - header layout string derived from CONFIG_FILE or -c file
    win_key_str         - window layout string derived from CONFIG_FILE or -c file
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
#define ORIG_BUF_LEN      16384     // Double ORIG_BUF_LEN  MAX_DOUBLE_TIMES before
#define MAX_DOUBLE_TIMES  3         // before it loops back around (3x: 32768 -> 65536 -> 131072)

/*
    scroll_buff_line_t
    --------
    - Ncurses WINDOW data buffer individual line data

    state->plugins[i]->win->buff_data->head_line, curr_line, tail_line

    prev    - previous node
    next    - next node

    ptr     - pointer to start of line in buffer
    len     - number of characters in line
    line    - line number
*/
typedef struct sbl_t {

    struct sbl_t *prev;
    struct sbl_t *next;

    char *ptr;
    int   len;
    int   line;

} scroll_buff_line_t;
                                        
/*
    buff_data_t
    --------
    - Data for each Ncurses WINDOW

    state->plugins[i]->win->buff_data

    code              - plugin code string
    changed           - boolean signal for buffer data to be reloaded before being displayed
    new_data          - boolean signal that new Dbg and/or Prg window data is in the debugger's 
                        buffer(s) and should be appended to the respective window_t buffer
    text_wrapped      - boolean signal for wrapped buffer text (all except Asm, Src)

    buff              - window data buffer
    buff_len          - buffer size
    buff_pos          - current index position of terminating null
    times_doubled     - number of times buffer size doubled from ORIG_BUF_LEN,
                        limited by MAX_DOUBLE_TIMES

    rows              - lines of data in buffer 
    scroll_row        - current scroll row, used to calculate curr_line
    scroll_col_offset - offset from first line characters
    max_chars         - number of characters in longest line, used for non-wrapped 
                        buffers (Asm, Src)

    head_line         - head node of scroll_buff_line_t linked list
    curr_line         - current node
    tail_line         - last node
*/
typedef struct {

    char   code [CODE_LEN+1];
    bool   changed;
    bool   new_data;
    bool   text_wrapped;

    char  *buff;  
    int    buff_len;
    int    buff_pos;
    int    times_doubled;

    int    rows;
    int    scroll_row;
    int    scroll_col_offset;
    int    max_chars;
    
    scroll_buff_line_t  *head_line;
    scroll_buff_line_t  *curr_line;
    scroll_buff_line_t  *tail_line;

} buff_data_t;

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
    path                - file path (if applicable)

    --- parent window ---

    rows                - parent rows
    cols                - columns
    y                   - top left screen row coordinate
    x                   - top left screen column coordinate
    border              - border settings required by wborder()

    --- topbar title subwindow if exists ---

    topbar_rows         - topbar subwindow rows
    topbar_cols         - columns
    topbar_y            - top left row coordinate relative to parent window
    topbar_x            - top left column coordinate    "
    topbar_title        - title string
                        
    --- data subwindow ---

    data_win_rows       - data subwindow rows
    data_win_cols       - columns
    data_win_y          - top left row coordinate relative to parent window
    data_win_x          - top left column coordinate     "
    data_win_mid_line   - middle row index

    buff_data           - buff_data_t pointer
*/
typedef struct {

    WINDOW       *WIN;
    WINDOW       *TWIN;
    WINDOW       *DWIN;

    int           index;
    char          code[CODE_LEN + 1];
    int           key;
    bool          has_topbar;

    int           rows;                   
    int           cols;                   
    int           y;                      
    int           x;                      
    int           border [8];

    int           topbar_rows;
    int           topbar_cols;
    int           topbar_y;
    int           topbar_x;
    char         *topbar_title;

    int           data_win_rows;
    int           data_win_cols;
    int           data_win_y;
    int           data_win_x;
    int           data_win_mid_line;

    buff_data_t  *buff_data;

} window_t;



/**********
  Debugger
 **********/

#define PIPE_READ         0
#define PIPE_WRITE        1
#define DEBUG_TITLE_LEN   8
#define PROGRAM_PATH_LEN  256
#define READER_BUF_LEN    8192
#define FUNC_LEN          128

enum { DEBUGGER_GDB, DEBUGGER_PDB };
enum { READER_RECEIVING, READER_DONE };

/*
    debugger_t
    -------
    state->debugger

    index                   - DEBUGGER_GDB, DEBUGGER_PDB, ...
    title                   - "gdb", "pdb", ...
    running                 - controls main() loop state
    stdin_pipe;             - debugger process input pipe
    stdout_pipe;            - debugger process output pipe

    prog_path               - binary, main script path buffer
    prog_update_time        - file's last update time (st_mtim.tv_sec), used to signal reload

    curr_func               - current function
                                        
    src_path_buffer         - path string of current source file
    src_path_len            - string buffer size
    src_path_pos            - null terminator index
    src_path_times_doubled  - times buffer size doubled
    src_path_changed        - source file path changed boolean

    reader_state            - READER_RECEIVING, RECEIVER_DONE
    reader_buffer           - read() output buffer for debugger process
                                        
    format_buffer           - misc buffer for formatting output
    format_len              - buffer size
    format_pos              - null terminator index
    format_times_doubled    - times buffer size doubled
    data_buffer             - data stream buffer
    data_len
    data_pos                
    data_times_doubled      
    cli_buffer              - cli stream buffer
    cli_len                 
    cli_pos                 
    cli_times_doubled       
    program_buffer          - program output buffer
    program_len             
    program_pos             
    program_times_doubled   
    async_buffer            - async (status) buffer
    async_len               
    async_pos               
    async_times_doubled     
*/
typedef struct {

    int     index;
    char    title [DEBUG_TITLE_LEN];
    bool    running;
    int     stdin_pipe;
    int     stdout_pipe;

    char    prog_path [PROGRAM_PATH_LEN];
    time_t  prog_update_time;

    int     curr_Src_line;
    int     curr_Asm_line;
    char    curr_func [FUNC_LEN];

    char   *src_path_buffer;
    int     src_path_len;
    int     src_path_pos;
    int     src_path_times_doubled;
    bool    src_path_changed;

    int     reader_state;
    char    reader_buffer [READER_BUF_LEN];

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

#define PLUGIN_CODE_LEN  3      // Asm, Src, ...
#define ESC              27

#define BEG_DATA         0      // display from beginning of data buffer
#define END_DATA         1      // display end of buffer data
#define ROW_DATA         3      // center data on state->debugger->curr_Src/Asm_line

/*
    plugin_t
    ------
    - Debugger command or window base struct
    - Plugin index corresponds to enum array value in plugins.h

    enum {
        Asm,
        ...
    }
    state->plugins[Asm]->code == "Asm"

    key         - plugin key binding
    code        - plugin code string
    title       - plugin header/window title string
    data_pos    - BEG_DATA, END_DATA, ROW_DATA
    has_window  - plugin has window in curr_layout
    win         - window_t pointer
*/
typedef struct {

    char      key;
    char      code [PLUGIN_CODE_LEN + 1];
    char     *title;
    int       data_pos;         
    bool      has_window;
    window_t *win;

} plugin_t;



/****************
  Persisted Data
 ****************/

#define PID_LEN          8
#define BREAK_PATH_LEN   128
#define BREAK_LINE_LEN   8
#define WATCH_LEN        84

/*
    breakpoint_t
    -------
    state->breakpoints

    index  - breakpoint index (created by debugger)
    path   - path string
    line   - line string
    next   - next breakpoint_t in linked list
*/
typedef struct breakpoint {

    char  index [8];
    char  path  [BREAK_PATH_LEN];
    char  line  [BREAK_LINE_LEN];
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



/***************
  Program State
 ***************/

#define CONFIG_PATH_LEN  128
#define DATA_PATH_LEN    CONFIG_PATH_LEN
#define INPUT_BUFF_LEN   4096

/*
    state_t
    -------
    - Base data struct that contains all other structs

    state->...

    num_plugins         - total number of plugins
    plugin_key_index    - array that matches key bindings to respective plugin's index in plugins.h
                          - plugin_key_index [key] = index
    config_path         - configuration file path set by CONFIG_FILE or -c flag
    data_path           - data persistence path set by PERSIST_FILE or -p flag
    input_buffer        - popup window input buffer

    plugins             - pointer array for plugin_t structs

    layouts             - linked list of layout_t structs
    curr_layout         - current layout_t struct
    header              - header Ncurses WINDOW

    debugger            - debugger_t struct
    command             - debugger command string array for execvp() in start_debugger()
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
    bool           new_run;

} state_t;



#endif

