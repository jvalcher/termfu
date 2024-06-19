#include <string.h>

#include "render_window_data.h"
#include "data.h"



/*
    Display file lines in Ncurses WINDOW shifted according to key, i.e.
    ---------
    - Text rendered inside WINDOW border
        - i.e. starting from column 1, row 1 -- not 0,0
    - Set key to 0 to display from first line

    - Keys:

        KEY_UP
        KEY_DOWN
        KEY_RIGHT
        KEY_LEFT

    - TODO: 
        KEY_HOME 
        KEY_NPAGE       - next page, page down
        KEY_PPAGE       - previous page, page up 
        KEY_END
*/
void render_window_data (window_t *win, int key) 
{
    // open output file
    win->file_ptr = fopen (win->file_path, "r");

#ifndef DEBUG

    char line[256];
    int  row = 1,
         col = 1,
         print_line,
         line_len,
         line_index,
         i, j;

    // clear window
    for (i = 1; i <= win->win_rows; i++) {
        for (j = 1; j <= win->win_cols; j++) {
            mvwaddch (win->win, i, j, ' ');
        }
    }

    // shift mid_line, first_char
    switch (key) {
        case FIRST_OPEN:
            break;
        case KEY_UP:
            win->win_mid_line = (win->win_mid_line <= win->file_min_mid) ? win->file_min_mid : win->win_mid_line - 1;
            break;
        case KEY_DOWN:
            win->win_mid_line = (win->win_mid_line >= win->file_max_mid) ? win->file_max_mid : win->win_mid_line + 1;
            break;
        case KEY_RIGHT:
            win->file_first_char = ((win->file_max_cols - win->file_first_char) > win->win_cols)
                ? win->file_first_char + 1
                : win->file_max_cols - win->win_cols;
            break;
        case KEY_LEFT:
            win->file_first_char = (win->file_first_char == 0) ? 0 : win->file_first_char - 1;
            break;
    }

    // get starting line
    print_line = win->win_mid_line - (win->win_rows / 2);

    // print lines to window
    for (i = 0; i < win->win_rows; i++) {

        // seek to beginning of line
        fseek (win->file_ptr, win->file_offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), win->file_ptr);
        line_len = strlen (line);

        // if line characters visible
        if (win->file_first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n')
                line_len -= 1;

            // calculate line length
            line_len = ((line_len - win->file_first_char) <= win->win_cols) 
                        ? line_len - win->file_first_char
                        : win->win_cols;

            // set line start index
            line_index = win->file_first_char;
        } 

        // if no characters visible
        else {
            line [0] = ' ';
            line_len = 1;
            line_index = 0;
        }

        // print line
        mvwaddnstr (win->win, row++, col, (const char *)(line + line_index), line_len);

        // break if end of file
        if (print_line > win->file_rows) 
            break;
    }
    //refresh();
    wrefresh(win->win);

#endif


#ifdef DEBUG

    int ch;
    while ((ch = fgetc (win->file_ptr)) != EOF) {
        putchar (ch);
    }

#endif


    fclose (win->file_ptr);
}
