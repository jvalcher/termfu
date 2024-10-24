#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <ctype.h>

#include "data.h"
#include "plugins.h"

static void format_window_data_Brk (state_t *state);
static void format_window_data_LcV (state_t *state);
static void format_window_data_Prg (state_t *state);
static void format_window_data_Src (state_t *state);
static void format_window_data_Wat (state_t *state);



int
format_window_data (int      plugin_index,
                    state_t *state)
{
    switch (plugin_index) {
        case Brk:
            format_window_data_Brk (state);
            break;
        case LcV:
            format_window_data_LcV (state);
            break;
        case Prg:
            format_window_data_Prg (state);
            break;
        case Src:
            format_window_data_Src (state);
            break;
        case Wat:
            format_window_data_Wat (state);
            break;
    }

    return A_OK;
}



static void
format_window_data_Brk (state_t *state)
{
    int       i, j, 
              ch,
              rows, cols;
    window_t *win;
    bool      line_num;

    win = state->plugins[Brk]->win;
    getmaxyx (win->DWIN, rows, cols);
    line_num = false;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;

            if (ch == '(') {
                
                // index
                ++j;
                wattron (win->DWIN, COLOR_PAIR(BREAK_INDEX_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                    if (ch != ')') {
                        mvwprintw (win->DWIN, i, j++, "%c", ch);
                    } else {
                        break;
                    }
                }
                wattroff (win->DWIN, COLOR_PAIR(BREAK_INDEX_COLOR));

                // file
                j += 2;
                wattron (win->DWIN, COLOR_PAIR(BREAK_FILE_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                    if (ch == ':') {
                        line_num = true;
                        break;
                    }
                    else if (ch == ' ') {
                        break;
                    } else {
                        mvwprintw (win->DWIN, i, j, "%c", ch);
                    }
                    ++j;
                }
                wattroff (win->DWIN, COLOR_PAIR(BREAK_FILE_COLOR));

                // line
                ++j;
                if (line_num) {
                    wattron (win->DWIN, COLOR_PAIR(BREAK_LINE_COLOR));
                    while (true) {
                        ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                        if (isalnum (ch)) {
                            mvwprintw (win->DWIN, i, j, "%c", ch);
                        } else {
                            break;
                        }
                        ++j;
                    }
                    wattroff (win->DWIN, COLOR_PAIR(BREAK_LINE_COLOR));
                    line_num = false;
                }
            }
        }
    }
    
    wrefresh (win->DWIN);
}



static void
format_window_data_LcV (state_t *state)
{
    int       i, j, k, 
              ch,
              rows, cols;
    window_t *win;

    // variable
    win = state->plugins[LcV]->win;
    getmaxyx (win->DWIN, rows, cols);
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            // check if line starts with (<index>)
            // i.e. identify start of line when text wrapped
            k = 1;
            if (  j == 0 &&
                 (ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT)   == '(' &&
                ((ch = mvwinch (win->DWIN, i, j + k++) & A_CHARTEXT)  > '0' && ch <= '9')) {

                while ((ch = mvwinch (win->DWIN, i, j + k++) & A_CHARTEXT) != ')') {
                    if (j+k >= cols) {
                        ch = '&';
                        break;
                    } else if (ch == ')') {
                        break;
                    } else if (ch < '0' || ch > '9') {
                        ch = '&';
                        break;
                    }
                }
                
                if (ch == ')') {

                    // index
                    ++j;
                    wattron (win->DWIN, COLOR_PAIR(WAT_INDEX_COLOR));
                    while (true) {
                        ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                        if (ch != ')') {
                            mvwprintw (win->DWIN, i, j++, "%c", ch);
                        } else {
                            break;
                        }
                    }
                    wattroff (win->DWIN, COLOR_PAIR(LOC_VAR_COLOR));

                    // variable
                    ++j;
                    wattron (win->DWIN, COLOR_PAIR(LOC_VAR_COLOR));
                    while (true) {
                        ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                        if (ch == '=' || j >= cols) {
                            break;
                        }
                        mvwprintw (win->DWIN, i, j++, "%c", ch);
                    }
                    wattroff (win->DWIN, COLOR_PAIR(LOC_VAR_COLOR));
                }
                    
            }
        }
    }

    wrefresh (win->DWIN);
}



static void
format_window_data_Prg (state_t *state)
{
    int       i, j, k,
              rows, cols,
              ch;
    window_t *win;

    win = state->plugins[Prg]->win;
    getmaxyx (win->DWIN, rows, cols);

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;

            k = 1;
            if ( j == 0 && ch == '<' &&
                (ch = mvwinch (win->DWIN, i, j+k++)) == 'N' &&
                (ch = mvwinch (win->DWIN, i, j+k++)) == 'e' &&
                (ch = mvwinch (win->DWIN, i, j+k++)) == 'w') 
            {
                ++j;
                wattron (win->DWIN, COLOR_PAIR(PROG_OUT_NEW_RUN_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                    if (ch != '>') {
                        mvwprintw (win->DWIN, i, j++, "%c", ch);
                    } else {
                        break;
                    }
                }
                wattroff (win->DWIN, COLOR_PAIR(PROG_OUT_NEW_RUN_COLOR));
            }
        }
    }

    wrefresh (win->DWIN);
}



// TODO: Replace source file line number with colored "b2" breakpoint index number

static void
format_window_data_Src (state_t *state)
{
    int           left_spaces,
                  right_spaces,
                  i, j, k,
                  ch,
                  rows, cols,
                  curr_line;
    char         *basefile,
                  line_buff [8];
    bool          is_curr_line;
    window_t     *win;

    win = state->plugins[Src]->win;

    // print current source code file in top bar
    basefile = basename (state->debugger->path_buffer);
    left_spaces = (win->topbar_cols - strlen (basefile)) / 2;
    right_spaces = win->topbar_cols - strlen (basefile) - left_spaces;
    left_spaces = left_spaces > 0 ? left_spaces : 0;
    right_spaces = right_spaces > 0 ? left_spaces : 0;

    wattron   (win->TWIN, COLOR_PAIR(TOPBAR_COLOR));
    mvwprintw (win->TWIN, 0, 0, "%*c%.*s%*c", left_spaces, ' ',
                                              win->topbar_cols, basefile,
                                              right_spaces, ' ');
    wattroff  (win->TWIN, COLOR_PAIR(TOPBAR_COLOR));
    wrefresh (win->TWIN);


    // highlight current line
    getmaxyx (win->DWIN, rows, cols);
    is_curr_line = false;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            if (is_curr_line) {
                if (i != curr_line) {
                    wattroff (win->DWIN, A_REVERSE);
                    is_curr_line = false;
                } else {
                    ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT; 
                    mvwprintw (win->DWIN, i, j, "%c", ch);
                }
            }

            // if at start of line
            if (j == 0) {

                k = 0;
                while (true) {
                    ch = mvwinch (win->DWIN, i, j++) & A_CHARTEXT; 
                    if (ch != ' ') {
                        line_buff [k++] = ch;
                    } else {
                        break;
                    }
                }
                line_buff [k] = '\0';

                if (atoi (line_buff) == state->debugger->curr_Src_line) {
                    wattron (win->DWIN, A_REVERSE);
                    is_curr_line = true;
                    curr_line = i;
                }
            }
        }
    }
    wattroff (win->DWIN, A_REVERSE);

    /*
    // replace line number with "b<breakpoint_index>"

    char index_buff [8];
    bool is_break_line;
    breakpoint_t *brkpnt;

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            if (j == 0) {

                // get line number
                k = 0;
                while (true) {
                    ch = mvwinch (win->DWIN, i, j++) & A_CHARTEXT; 
                    if (ch != ' ') {
                        line_buff [k++] = ch;
                    } else {
                        break;
                    }
                }
                line_buff [k] = '\0';

                // determine if current line a breakpoint
                is_break_line = false;
                brkpnt = state->breakpoints;
                do {
                    if (strcmp (basename (state->debugger->path_buffer), brkpnt->path)) {
                        if (strcmp (brkpnt->line, line_buff) == 0) {
                            strcpy (index_buff, brkpnt->index);
                            is_break_line = true;
                            break;
                        }
                    }
                    brkpnt = brkpnt->next;
                } while (brkpnt != NULL);

                if (is_break_line) {
                    wattron   (win->DWIN, COLOR_PAIR(SRC_BREAK_LINE_COLOR));
                    mvwprintw (win->DWIN, i, 0, "b%s", index_buff);
                    wattroff  (win->DWIN, COLOR_PAIR(SRC_BREAK_LINE_COLOR));
                } else {
                    mvwprintw (win->DWIN, i, 0, "%s", line_buff);
                }

                //next line
                ++i;
            }
        }
    }
    */

    wrefresh (win->DWIN);
}



static void
format_window_data_Wat (state_t *state)
{
    int       i, j, 
              ch,
              rows, cols;
    window_t *win;

    win = state->plugins[Wat]->win;
    getmaxyx (win->DWIN, rows, cols);

    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;

            if (j == 0 && ch == '(') {
                
                // index
                ++j;
                wattron (win->DWIN, COLOR_PAIR(WAT_INDEX_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                    if (ch != ')') {
                        mvwprintw (win->DWIN, i, j++, "%c", ch);
                    } else {
                        break;
                    }
                }
                wattroff (win->DWIN, COLOR_PAIR(LOC_VAR_COLOR));

                // variable
                j += 2;
                wattron (win->DWIN, COLOR_PAIR(WAT_VAR_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j) & A_CHARTEXT;
                    if (ch == '=') {
                        break;
                    } else {
                        mvwprintw (win->DWIN, i, j++, "%c", ch);
                    }
                }
                wattroff (win->DWIN, COLOR_PAIR(WAT_VAR_COLOR));

            }
        }
    }
    
    wrefresh (win->DWIN);
}

