/*
    Ncurses window display utilities
    ------------------------
    render_window
    center_str
*/

#ifndef window_h
#define window_h


/* 
    Color pair identifiers for that <COLOR> font on 
    black background used in enable_colors()
*/
#define RED     20
#define GREEN   21
#define YELLOW  22
#define BLUE    23
#define MAGENTA 24
#define CYAN    25
#define WHITE   26


/* 
   Display program title
*/
void render_window();


#endif
