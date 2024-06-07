
/*
    Ncurses pad tests
*/
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

#include "data.h"

/*
    TODO:  
    -----
    - Check if current file_offsets is large enough to store new file offsets
*/


void get_num_file_rows_cols (window_t* win);
void display_lines (window_t* win, int key);



int main (void) 
{
    int   i,
          ch,
          begin_y,
          begin_x;

    // allocate
    data_t *data = (data_t*) malloc (sizeof (data_t));
    window_t *window = (window_t*) malloc (sizeof (window_t));
    debug_state_t *dstate = (debug_state_t*) malloc (sizeof (debug_state_t));

    data->window = window;
    data->debug_state = dstate;

    // open source file
    strncpy (window->file_path, "./file.txt", sizeof (window->file_path) - 1);
    window->file_ptr = fopen(window->file_path, "r");
    if (window->file_ptr == NULL) {
        perror ("Failed to open source file");
        return 1;
    }

    // get number of file rows and max line length
    get_num_file_rows_cols (window);

    // get file line offsets
        // allocate array
    window->file_offsets = malloc ((size_t) window->file_rows * sizeof(long int));
    if (window->file_offsets == NULL) {
        perror ("Failed to allocate offsets array");
        return 1;
    }
        // add offsets to array
    window->file_offsets [0] = 0;
    for (i = 1; i < window->file_rows; i++) {
        while ((ch = fgetc (window->file_ptr)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            window->file_offsets [i] = ftell (window->file_ptr);
    }

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    refresh();

    // create window
    window->win_rows = 12;
    window->win_cols = 48;
    begin_y = 2;
    begin_x = 2;
    window->win = newwin (window->win_rows, window->win_cols, begin_y, begin_x);
    box (window->win, 0, 0);
    keypad(stdscr, TRUE);
    wrefresh (window->win);

    // calculate output window values
    window->win_rows -= 2;
    window->win_cols -= 2;
    window->file_min_mid = (window->win_rows / 2) + 1;
    window->file_max_mid = window->file_rows - ((window->win_rows - 1) / 2);
    window->win_mid_line = window->file_min_mid;
    window->file_first_char = 0;
    
    // display starting lines
    display_lines(window, 0);

    // scroll loop
    while ((ch = getch()) != 'q') { // Press 'q' to quit
        switch (ch) {
            case KEY_UP:
                display_lines(window, KEY_UP);
                break;
            case KEY_DOWN:
                display_lines(window, KEY_DOWN);
                break;
            case KEY_RIGHT:
                display_lines(window, KEY_RIGHT);
                break;
            case KEY_LEFT:
                display_lines(window, KEY_LEFT);
                break;
        }
    }

    keypad (window->win, FALSE);
    curs_set(1);
    endwin ();

    free (data->window->file_offsets);
    free (data->window);
    free (data->debug_state);
    free (data);

    return 0;
}


void get_num_file_rows_cols (window_t *win) {
    win->file_rows = 0;
    win->file_max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), win->file_ptr) != NULL) {
        line_len = strlen(line);
        if (win->file_max_cols < line_len) {
            win->file_max_cols = line_len + 1;
        }
        win->file_rows += 1;
    }
    rewind (win->file_ptr);
}


/*
    Display file lines shifted according to key, i.e.

        KEY_UP
        KEY_DOWN
        KEY_RIGHT
        KEY_LEFT
*/
void display_lines (window_t *win, int key) 
{
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
        case 0:
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

    // calculate first line
    print_line = win->win_mid_line - (win->win_rows / 2);

    // print lines
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
        if (print_line > win->file_rows) break;
    }

    // Update screen
    //refresh();
    wrefresh(win->win);
}

