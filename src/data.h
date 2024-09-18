 
#ifndef DATA_H
#define DATA_H

#include <ncurses.h>
#include <time.h>



#define PROGRAM_NAME          "termfu"
#define CONFIG_FILE           ".termfu" 
#define PERSIST_FILE          ".termfu_data"
#define CONFIG_COMMAND_LABEL  "command"
#define CONFIG_PLUGINS_LABEL  "plugins"
#define CONFIG_LAYOUTS_LABEL  "layout"
#define DEBUG_OUT_FILE        "debug.out"



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



/*********
  Layouts
 *********/

#define FIRST_LAYOUT            "FiRsT_lAyOuT"
#define MAX_CONFIG_CATEG_LEN    128
#define MAX_CONFIG_LABEL_LEN    128
#define MAX_KEY_STR_LEN         256
#define MAX_TITLE_LEN           48
#define MAX_ROW_SEGMENTS        24      // per layout
#define MAX_COL_SEGMENTS        24

typedef struct layout {

    char            label [MAX_CONFIG_LABEL_LEN];
    char            hdr_key_str [MAX_KEY_STR_LEN];
    char            win_key_str [MAX_KEY_STR_LEN];
    int             num_hdr_key_rows;
    char          **win_matrix;
    int             row_ratio;
    int             col_ratio;
    struct layout  *next;

} layout_t;



/*********
  Ncurses window data
 *********/

#define NO_DATA_MSG    "Not supported by "
#define FILE_PATH_LEN  256
#define ADDRESS_LEN    48
#define FUNC_LEN       48
#define Asm_BUF_LEN    131072
#define Brk_BUF_LEN    4096
#define Dbg_BUF_LEN    32768
#define LcV_BUF_LEN    16384
#define Prg_BUF_LEN    32768
#define Reg_BUF_LEN    16384
#define Stk_BUF_LEN    16384
#define Wat_BUF_LEN    4096

typedef struct {

    char  *buff;  
    int    buff_len;
    int    buff_pos;
    bool   changed;
    int    rows;
    int    max_cols;
    int    scroll_row;
    int    scroll_col;
    bool   new_data;

} buff_data_t;

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
    char   formatted_path [FILE_PATH_LEN];
    bool   path_changed;
    char   addr [ADDRESS_LEN];
    int    addr_len;
    int    addr_pos;
    char   func [FUNC_LEN];
    int    func_len;
    int    func_pos;

} file_data_t;

typedef struct {

    WINDOW       *WIN;
    WINDOW       *IWIN;
    WINDOW       *DWIN;

    int           key;
    char          code[4];
    bool          has_topbar;
    bool          has_data_buff;     // as opposed to file

    // parent window
    int           rows;                   
    int           cols;                   
    int           y;                      
    int           x;                      
    int           border [8];

    // topbar title
    int           topbar_rows;
    int           topbar_cols;
    int           topbar_y;
    int           topbar_x;
    char         *topbar_title;

    // data window
    int           data_win_rows;
    int           data_win_cols;
    int           data_win_y;
    int           data_win_x;
    int           data_win_mid_line;

    // buffer, file data
    buff_data_t  *buff_data;
    file_data_t  *file_data;

} window_t;



/**********
  Debugger
 **********/

#define PIPE_READ        0
#define PIPE_WRITE       1
#define DEBUG_TITLE_LEN  8
#define PROGRAM_PATH_LEN 256
#define READER_BUF_LEN   8192
#define FORMAT_BUF_LEN   65536
#define DATA_BUF_LEN     65536
#define CLI_BUF_LEN      65536
#define PROGRAM_BUF_LEN  65536
#define ASYNC_BUF_LEN    65536 

enum { DEBUGGER_GDB, DEBUGGER_PDB };
enum { READER_RECEIVING, READER_DONE };

typedef struct {

    int     index;
    char    title [DEBUG_TITLE_LEN];
    bool    running;
    char    prog_path [PROGRAM_PATH_LEN];
    time_t  prog_update_time;

    int     stdin_pipe;
    int     stdout_pipe;

    char    format_buffer  [FORMAT_BUF_LEN];
    char    data_buffer    [DATA_BUF_LEN];
    char    cli_buffer     [CLI_BUF_LEN];
    char    program_buffer [PROGRAM_BUF_LEN];
    char    async_buffer   [ASYNC_BUF_LEN];

} debugger_t;

typedef struct {

    int    state;
    char   output_buffer [READER_BUF_LEN];
    char  *cli_buffer_ptr;
    char  *program_buffer_ptr;
    char  *data_buffer_ptr;
    char  *async_buffer_ptr;
    int    plugin_index;

} reader_t;



/*********
  Plugins
 *********/

#define PLUGIN_CODE_LEN  3

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

#define BREAK_LEN       256
#define WATCH_LEN       84
#define INPUT_BUFF_LEN  4096

typedef struct breakpoint {

    char  index [8];
    char  path_line [BREAK_LEN];
    struct breakpoint *next;

} breakpoint_t;

typedef struct watchpoint {

    int   index;
    char  var   [WATCH_LEN];
    char  value [WATCH_LEN];
    struct watchpoint *next;

} watchpoint_t;

typedef struct {

    int            num_plugins;
    int           *plugin_key_index;
    char           input_buffer [INPUT_BUFF_LEN];

    layout_t      *layouts;
    layout_t      *curr_layout;
    WINDOW        *header;

    debugger_t    *debugger;
    char         **command;

    plugin_t     **plugins;

    watchpoint_t  *watchpoints;
    breakpoint_t  *breakpoints;

} state_t;



#endif
