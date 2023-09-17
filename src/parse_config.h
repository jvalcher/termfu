
/*
    Apply settings in configuration file
*/

#ifndef PARSE_CONFIG_H
#define PARSE_CONFIG_H


#define MAX_WINDOW_TYPES    24
#define MAX_LAYOUTS         10
#define MAX_TITLE_LEN       20
#define MAX_WINDOWS         20      // per layout
#define MAX_Y_SEGMENTS      10      // per layout
#define MAX_X_SEGMENTS      10      // per layout


/*
    Window symbol to title string array
*/
char *layout_titles [MAX_WINDOW_TYPES] = {'\0'};
layout_titles ['a'] = "Assembly";
layout_titles ['b'] = "Breakpoints";
layout_titles ['c'] = "Commands";
layout_titles ['l'] = "Local vars";
layout_titles ['r'] = "Registers";
layout_titles ['s'] = "Source";
layout_titles ['t'] = "Status";
layout_titles ['w'] = "Watches";


/*
    Window
    ------
    Define the symbol, title, width, height, position for a 
    single layout
*/
struct window {
    char symbol;                        // window symbol
    char title [MAX_TITLE_LEN];     // title string
    int rows;                       // height in rows
    int cols;                       // width in columns
    int y;                          // top left corner y
    int x;                          // top left corner x
};


/*
    Layouts
    -----------
    num_layouts     == total number of layouts
    labels [i]      == array of label strings  ([layout:<label>])
    num_windows [i] == number of windows in each layout
    windows [i]     == array of pointers to window structs
*/
struct layouts {
    int num_layouts;
    char labels [MAX_TITLE_LEN];
    int num_windows [MAX_LAYOUTS];
    struct window *windows [MAX_WINDOWS];
};


/*
    Parse config_file_data struct (get_config.h) settings
    Pass resulting data into above configuration structs
    Add to respective array
*/
void parse_config ();


#endif
