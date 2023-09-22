
/*
   Render Ncurses windows
*/


#ifndef render_screen_h
#define render_screen_h


// Window symbols, titles
#define NUM_SYMBOLS     52
#define NUM_WINDOWS     8
#define MAX_TITLE_LEN   20
char win_symbols [] = {'a', 'b', 'c', 'l', 'r', 's', 't', 'w'};
char *win_titles [] = {
    "Assembly",         // a
    "Breakpoints",      // b
    "Commands",         // c
    "Local vars",       // l
    "Registers",        // r
    "Source",           // s
    "Status",           // t
    "Watches"           // w
};


// Window data
struct window_data {
    char symbols [NUM_SYMBOLS];
    char titles  [NUM_SYMBOLS] [MAX_TITLE_LEN];
};


// Return index used in window_data struct 
// from window symbol character (A-Z, a-z)
//
//     char  ascii  index
//     ----  -----  -----
//     'A'    65      0
//     'Z'    90     25
//     'a'    97     26
//     'z'    122    51
//
//      get_win_index ('B') --> 1
//
int get_win_index (char symbol);

/* 
   Render main window
*/
void render_screen();


#endif
