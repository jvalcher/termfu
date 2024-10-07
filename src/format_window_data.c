#include <string.h>
#include <stdlib.h>
#include <libgen.h>
#include <ctype.h>

#include "data.h"
#include "plugins.h"

static void format_window_data_Asm (state_t *state);
static void format_window_data_Brk (state_t *state);
static void format_window_data_LcV (state_t *state);
static void format_window_data_Prg (state_t *state);
static void format_window_data_Src (state_t *state);
static void format_window_data_Wat (state_t *state);



void
format_window_data (int      plugin_index,
                    state_t *state)
{
    switch (plugin_index) {
        case Asm:
            format_window_data_Asm (state);
            break;
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
}



static void
format_window_data_Asm (state_t *state)
{
    int       i, j, 
              m, n,
              ch,
              rows, cols;
    size_t    k, si;
    window_t *win;
    char     *needle;

    // highlight current hex address
    si = 0;
    win = state->plugins[Asm]->win;
    getmaxyx (win->DWIN, rows, cols);
    needle = state->plugins[Src]->win->src_file_data->addr;
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            ch = mvwinch (win->DWIN, i, j);
            if ((char) ch == needle [si]) {
                if (si == 0) {
                    m = i;
                    n = j;
                }
                si += 1;
                if (si == strlen (needle)) {
                    wattron (win->DWIN, A_REVERSE);
                    for (k = 0; k < strlen (needle); k++) {
                        mvwprintw (win->DWIN, m, n + k, "%c", needle[k]);
                    }
                    wattroff (win->DWIN, A_REVERSE);
                }
            } 
            else {
                si = 0;
            }
        }
    }
    wrefresh (win->DWIN);
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

            ch = mvwinch (win->DWIN, i, j);

            if (ch == '(') {
                
                // index
                ++j;
                wattron (win->DWIN, COLOR_PAIR(BREAK_INDEX_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j);
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
                    ch = mvwinch (win->DWIN, i, j);
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
                        ch = mvwinch (win->DWIN, i, j);
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
                 (ch = mvwinch (win->DWIN, i, j))   == '(' &&
                ((ch = mvwinch (win->DWIN, i, j + k++))  > '0' && ch <= '9')) {

                while ((ch = mvwinch (win->DWIN, i, j + k++)) != ')') {
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
                        ch = mvwinch (win->DWIN, i, j);
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
                        ch = mvwinch (win->DWIN, i, j);
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

            ch = mvwinch (win->DWIN, i, j);

            k = 1;
            if ( j == 0 && ch == '<' &&
                (ch = mvwinch (win->DWIN, i, j+k++)) == 'N' &&
                (ch = mvwinch (win->DWIN, i, j+k++)) == 'e' &&
                (ch = mvwinch (win->DWIN, i, j+k++)) == 'w') 
            {
                ++j;
                wattron (win->DWIN, COLOR_PAIR(PROG_OUT_NEW_RUN_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j);
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
                  line_buff [8],
                  index_buff [8];
    bool          is_curr_line,
                  is_break_line;
    window_t     *win;
    breakpoint_t *brkpnt;

    win = state->plugins[Src]->win;

    // print current source code file in top bar
    basefile = basename (win->src_file_data->path);
    left_spaces = (win->topbar_cols - strlen (basefile)) / 2;
    right_spaces = win->topbar_cols - strlen (basefile) - left_spaces;
    left_spaces = left_spaces > 0 ? left_spaces : 0;
    right_spaces = right_spaces > 0 ? left_spaces : 0;

    wattron   (win->TWIN, COLOR_PAIR(WINDOW_INPUT_TITLE_COLOR));
    mvwprintw (win->TWIN, 0, 0, "%*c%.*s%*c", left_spaces, ' ',
                                              win->topbar_cols, basefile,
                                              right_spaces, ' ');
    wattroff  (win->TWIN, COLOR_PAIR(WINDOW_INPUT_TITLE_COLOR));
    wrefresh (win->TWIN);


    // highlight line number, current line
    if (state->breakpoints != NULL) {

        getmaxyx (win->DWIN, rows, cols);
        is_curr_line = false;
        is_break_line = false;
        for (i = 0; i < rows; i++) {
            for (j = 0; j < cols; j++) {

                if (is_curr_line) {
                    if (i != curr_line) {
                        wattroff (win->DWIN, A_REVERSE);
                        is_curr_line = false;
                    } else {
                        ch = mvwinch (win->DWIN, i, j); 
                        mvwprintw (win->DWIN, i, j, "%c", ch);
                    }
                }

                if (j == 0) {

                    k = 0;
                    while (true) {
                        ch = mvwinch (win->DWIN, i, j++); 
                        if (ch != ' ') {
                            line_buff [k++] = ch;
                        } else {
                            break;
                        }
                    }
                    line_buff [k] = '\0';

                    // determine if current line a breakpoint
                    brkpnt = state->breakpoints;
                    do {
                        if (strcmp (basename (win->src_file_data->path), brkpnt->path)) {
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
                        is_break_line = false;
                    } else {
                        wattron   (win->DWIN, COLOR_PAIR(SRC_LINE_COLOR));
                        mvwprintw (win->DWIN, i, 0, "%s", line_buff);
                        wattroff  (win->DWIN, COLOR_PAIR(SRC_LINE_COLOR));
                    }

                    if (atoi (line_buff) == win->src_file_data->line) {
                        wattron (win->DWIN, A_REVERSE);
                        is_curr_line = true;
                        curr_line = i;
                    }
                }
            }
        }

        wattroff (win->DWIN, COLOR_PAIR(SRC_LINE_COLOR));
        wattroff (win->DWIN, A_REVERSE);
        wrefresh (win->DWIN);
    }

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

            ch = mvwinch (win->DWIN, i, j);

            if (j == 0 && ch == '(') {
                
                // index
                ++j;
                wattron (win->DWIN, COLOR_PAIR(WAT_INDEX_COLOR));
                while (true) {
                    ch = mvwinch (win->DWIN, i, j);
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
                    ch = mvwinch (win->DWIN, i, j);
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

