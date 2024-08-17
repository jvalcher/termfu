#ifndef GET_POPUP_WINDOW_INPUT_H
#define GET_POPUP_WINDOW_INPUT_H

#include "../data.h"



/*
    Open popup window with <prompt> and save typed string into <buffer>
*/
void  get_popup_window_input  (char *prompt, char *buffer);



/*
    Close currently open popup window
*/
void  close_popup_window  (void);



#endif
