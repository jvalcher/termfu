
/*
    Ncurses pad tests
*/
#include <stdio.h>
#include <string.h>
#include <ncurses.h>
#include <stdlib.h>

/*
    TODO:  
    - implement right, left scroll
    - mvwaddnstr from (line + offset) to n
        - border line offset
            - y = 1, 2, ...  
            - x = 1 -> (cols - 2)
    - allocate int array for each line's offset to use in fseek
*/


typedef struct window_info {

    WINDOW* win;
    int win_rows;
    int win_cols;

    const char* file_name;
    FILE* file;
    int mid_line;
    int first_char;
    int min_mid;
    int max_mid;
    int file_rows;
    int file_max_cols;
    unsigned long int *offsets;

} window_info;

void get_num_file_rows_cols (window_info* win_info);
void display_lines (window_info* win_info, int key);


int main (void) 
{
    window_info wi;
    int   i,
          ch,
          begin_y,
          begin_x;

    // open source file
    wi.file_name = "file.txt";
    wi.file = fopen(wi.file_name, "r");
    if (wi.file == NULL) {
        perror ("Failed to open source file");
        return 1;
    }

    // get number of file rows and max line length
    get_num_file_rows_cols(&wi);

    // get file line offsets
        // allocate array
    wi.offsets = malloc ((size_t) wi.file_rows * sizeof(long int));
    if (wi.offsets == NULL) {
        perror ("Failed to allocate offsets array");
        return 1;
    }
        // add offsets to array
    wi.offsets [0] = 0;
    for (i = 1; i < wi.file_rows; i++) {
        while ((ch = fgetc (wi.file)) != '\n' && ch != EOF) {}
        if (ch == '\n')
            wi.offsets [i] = ftell (wi.file);
    }

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    refresh();

    // create window
    wi.win_rows = 12;
    wi.win_cols = 48;
    begin_y = 2;
    begin_x = 2;
    wi.win = newwin (wi.win_rows, wi.win_cols, begin_y, begin_x);
    box (wi.win, 0, 0);
    keypad(stdscr, TRUE);
    wrefresh (wi.win);

    // calculate window values
    wi.win_rows -= 2;
    wi.win_cols -= 2;
    wi.min_mid = (wi.win_rows / 2) + 1;
    wi.max_mid = wi.file_rows - ((wi.win_rows - 1) / 2);
    wi.mid_line = wi.min_mid;
    wi.first_char = 0;
    
    // display starting lines
    display_lines(&wi, 0);

    // scroll loop
    while ((ch = getch()) != 'q') { // Press 'q' to quit
        switch (ch) {
            case KEY_UP:
                display_lines(&wi, KEY_UP);
                break;
            case KEY_DOWN:
                display_lines(&wi, KEY_DOWN);
                break;
            case KEY_RIGHT:
                display_lines(&wi, KEY_RIGHT);
                break;
            case KEY_LEFT:
                display_lines(&wi, KEY_LEFT);
                break;
        }
    }

    keypad (wi.win, FALSE);
    curs_set(1);
    endwin ();
    fclose (wi.file);
    free   (wi.offsets);
    return 0;
}


void get_num_file_rows_cols (window_info* win_info) {
    win_info->file_rows = 0;
    win_info->file_max_cols = 0;
    char  line [512];
    int line_len;
    while (fgets(line, sizeof(line), win_info->file) != NULL) {
        line_len = strlen(line);
        if (win_info->file_max_cols < line_len) {
            win_info->file_max_cols = line_len + 1;
        }
        win_info->file_rows += 1;
    }
    rewind (win_info->file);
}


/*
    Display file lines shifted according to key, i.e.

        KEY_UP
        KEY_DOWN
        KEY_RIGHT
        KEY_LEFT
*/
void display_lines (window_info* wi, int key) 
{
    char line[256];
    int  row = 1,
         col = 1,
         print_line,
         line_len,
         line_index,
         i, j;

    // clear window
    for (i = 1; i <= wi->win_rows; i++) {
        for (j = 1; j <= wi->win_cols; j++) {
            mvwaddch (wi->win, i, j, ' ');
        }
    }

    // shift mid_line, first_char
    switch (key) {
        case 0:
            break;
        case KEY_UP:
            wi->mid_line = (wi->mid_line <= wi->min_mid) ? wi->min_mid : wi->mid_line - 1;
            break;
        case KEY_DOWN:
            wi->mid_line = (wi->mid_line >= wi->max_mid) ? wi->max_mid : wi->mid_line + 1;
            break;
        case KEY_RIGHT:
            wi->first_char = ((wi->file_max_cols - wi->first_char) > wi->win_cols)
                ? wi->first_char + 1
                : wi->file_max_cols - wi->win_cols;
            break;
        case KEY_LEFT:
            wi->first_char = (wi->first_char == 0) ? 0 : wi->first_char - 1;
            break;
    }

    // calculate first line
    print_line = wi->mid_line - (wi->win_rows / 2);

    // print lines
    for (i = 0; i < wi->win_rows; i++) {

        // seek to beginning of line
        fseek (wi->file, wi->offsets[print_line++ - 1], SEEK_SET);

        // get line
        fgets (line, sizeof (line), wi->file);
        line_len = strlen (line);

        // if line characters visible
        if (wi->first_char <= line_len) {

            // remove newline
            if (line [line_len - 1] == '\n')
                line_len -= 1;

            // calculate line length
            line_len = ((line_len - wi->first_char) <= wi->win_cols) 
                        ? line_len - wi->first_char
                        : wi->win_cols;

            // set line start index
            line_index = wi->first_char;
        } 

        // if no characters visible
        else {
            line [0] = ' ';
            line_len = 1;
            line_index = 0;
        }

        // print line
        mvwaddnstr (wi->win, row++, col, (const char *)(line + line_index), line_len);

        // break if end of file
        if (print_line > wi->file_rows) break;
    }

    // Update screen
    refresh();
    wrefresh(wi->win);
}

