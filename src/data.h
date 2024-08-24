 
#ifndef DATA_H
#define DATA_H

#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <semaphore.h>



/******
  Main
 ******/

#define PROGRAM_NAME     "term_debug"       // TODO: rename program
#define DATA_DIR_PATH    ".local/share/term_debug"
#define DEBUG_OUT_PATH   "debug.out"
#define PARENT_PROCESS   0
#define CHILD_PROCESS    1



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

#define CONFIG_FILE             ".term_debug"
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
    header            - Ncurses WINDOW struct for header
    hdr_key_str       - header plugin key string  ("ssb\nssw\nccr\n")
    win_key_str       - window plugin key string  ("ssb\nssw\nccr\n")
    num_hdr_key_rows  - number of header rows needed for key strings
    win_matrix        - window key segment* matrix
    row_ratio         - y segment ratio for layout matrix
    col_ratio         - x segment ratio for layout matrix
    next              - next layout_t struct
*/
typedef struct layout {

    char            label [MAX_CONFIG_LABEL_LEN];        // TODO: allocate
    WINDOW         *header;
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

#define MAX_LINES       128
#define FILE_PATH_LEN   128
#define WIN_INPUT_LEN   128
#define DEBUG_BUF_LEN   4096

typedef struct {

    char  buff [DEBUG_BUF_LEN];
    bool  changed;
    int   rows;
    int   max_cols;
    int   scroll_row;
    int   scroll_col;

} buff_data_t;

typedef struct {

    FILE  *ptr;
    char   path [FILE_PATH_LEN];
    bool   path_changed;
    int    line;
    int    line_num_digits;
    int    first_char;
    int    rows;
    int    max_cols;
    int    min_mid;
    int    max_mid;
    long  *offsets;

} file_data_t;

typedef struct {

    WINDOW  *WIN;
    WINDOW  *IWIN;
    WINDOW  *DWIN;

    char     code[4];
    bool     selected;
    bool     has_input;
    bool     has_data_buff;     // as opposed to file

    // parent window
    int      rows;                   
    int      cols;                   
    int      y;                      
    int      x;                      
    int      border [8];

    // input line
    int      input_rows;
    int      input_cols;
    int      input_y;
    int      input_x;
    char    *input_title;
    char     input_buffer [WIN_INPUT_LEN];

    // data window
    int      data_win_rows;
    int      data_win_cols;
    int      data_win_y;
    int      data_win_x;
    int      data_win_mid_line;

    // buffer, file data
    buff_data_t *buff_data;
    file_data_t *file_data;

} window_t;



/**********
  Debugger
 **********/

#define PIPE_READ            0
#define PIPE_WRITE           1
#define READER_MSG_MAX_LEN  24
#define CMD_MAX_LEN         256

enum { DEBUGGER_GDB };
enum { READER_RECEIVING, READER_DONE };

typedef struct {

    int     curr;
    bool    running;
    char  **cmd;
    char   *prog_path;

    int     stdin_pipe;
    int     stdout_pipe;

    char    format_buffer  [DEBUG_BUF_LEN];
    char    cli_buffer     [DEBUG_BUF_LEN];
    char    program_buffer [DEBUG_BUF_LEN];
    char    data_buffer    [DEBUG_BUF_LEN];
    char    async_buffer   [DEBUG_BUF_LEN];

} debugger_t;

typedef struct {

    int   state;
    char  output_line_buffer [DEBUG_BUF_LEN];
    char *cli_buffer_ptr;
    char *program_buffer_ptr;
    char *data_buffer_ptr;
    char *async_buffer_ptr;
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
#define PLUGIN_CODE_LEN  3

typedef struct {

    int       index;
    char      key;
    char      code [4];
    char     *title;
    bool      has_window;
    window_t *win;

} plugin_t;



/*******
  State
 *******/

#define INPUT_BUFF_LEN  256

typedef struct {

    int           num_plugins;
    int          *plugin_key_index;
    char          input_buffer [INPUT_BUFF_LEN];

    debugger_t   *debugger;

    layout_t     *layouts;
    plugin_t    **plugins;
    WINDOW       *header;

} state_t;



#endif
