
/*
   render ncurses screen
*/


#ifndef render_screen_h
#define render_screen_h


/* 
    Color pair identifiers
    ----------------------
    - <font>_<background>
    - created in enable_colors()
*/
#define RED_BLACK       20
#define GREEN_BLACK     21
#define YELLOW_BLACK    22
#define BLUE_BLACK      23
#define MAGENTA_BLACK   24
#define CYAN_BLACK      25
#define WHITE_BLACK     26
#define WHITE_BLUE      27


/* 
   Render main window
*/
void render_screen();


#endif
