
#ifndef COLORS_H
#define COLORS_H

/*
    Color settings
*/
#define BORDER_COLOR        BLUE_BLACK
#define TITLE_COLOR         CYAN_BLACK


/* 
    Color pair identifiers
    ----------------------
    - <font>_<background>
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
   Create font_background color pairs
   ---------------------
   - check current terminal's color capability with:  
        $ make colors
*/
void  create_colors();


#endif
