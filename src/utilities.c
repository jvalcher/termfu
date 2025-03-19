#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "utilities.h"
#include "error.h"
#include "plugins.h"
#include "data.h"
#include "parse_debugger_output.h"
#include "persist_data.h"

FILE    *debug_out_ptr = NULL;
state_t *state_ptr = NULL;
bool     program_cleaned_up = false;



void
logd (const char *formatted_string, ...)
{
    if (debug_out_ptr == NULL) {
        if ((debug_out_ptr = fopen (DEBUG_OUT_FILE, "w")) == NULL)
            pfeme_errno ("Failed to open debug out file \"%s\"", DEBUG_OUT_FILE);
    }

    va_list args;
    va_start (args, formatted_string);
    vfprintf (debug_out_ptr, formatted_string, args);
    va_end (args);
}



int
free_nc_window_data (state_t *state)
{
    for (int i = 0; i < state->num_plugins; i++) {

        if (state->plugins[i]->has_window) {

            if (state->plugins[i]->win->TWIN != NULL)
                if (delwin (state->plugins[i]->win->TWIN) == FAIL)
                    pfemr ("Unable to delete TWIN (index: %d, code: %s)", i, get_plugin_code (i));

            if (state->plugins[i]->win->DWIN != NULL)
                if (delwin (state->plugins[i]->win->DWIN) == FAIL)
                    pfemr ("Unable to delete DWIN (index: %d, code: %s)", i, get_plugin_code (i));

            if (state->plugins[i]->win->WIN != NULL)
                if (delwin (state->plugins[i]->win->WIN) == FAIL)
                    pfemr ("Unable to delete WIN (index: %d, code: %s)", i, get_plugin_code (i));

            refresh ();
        }
    }
    return A_OK;
}



void
clean_up (int cause)
{
    if (program_cleaned_up == false) {
        program_cleaned_up = true;

        // exit ncurses
        if (state_ptr != NULL) {

            // header subwindow
            if (state_ptr->header != NULL)
                if (delwin (state_ptr->header) == ERR)
                    pfem ("Failed to delete ncurses header subwindow");

            // data windows
            if (free_nc_window_data (state_ptr) == FAIL)
                pfem (ERR_NC_FREE);

            curs_set (1);
            endwin ();

            // persist breakpoint, watchpoint data
            if (persist_data (state_ptr) == FAIL)
                pfem (ERR_PERSIST);
        }

        // close DEBUG_OUT_FILE
        if (debug_out_ptr != NULL)
            if (fclose (debug_out_ptr) != 0)
                pfem_errno (ERR_DBG_FCLOSE);

        // print error header
        if (cause == PROG_ERROR)
            fprintf (stderr,  RED "ERROR" CYAN " :: " R "termfu exited\n");
    }
}



char*
concatenate_strings_impl (int max_strs, ...)
{
    int      str_len,
             str_count;
    char    *sub_str,
            *str;
    va_list strs;
    
    // calculate total string length
    str_len = 0;
    str_count = 0;
    va_start (strs, max_strs);
    for (str = va_arg(strs, char*);
         str != NULL;
         str = va_arg(strs, char*))
    {
        str_len += strlen (str);
        if (++str_count >= max_strs)
            pfemn ("Max strings exceeded");
    }
    va_end (strs);

    // allocate
    if ((str = (char*) malloc (str_len + 1)) == NULL)
        pfemn_errno  ("Failed to allocate space for string (length: %d)", str_len);
    str [0] = '\0';

    // create string
    va_start (strs, max_strs);
    for (sub_str = va_arg(strs, char*);
         sub_str != NULL;
         sub_str = va_arg(strs, char*))
    {
        strncat (str, sub_str, str_len - strlen(str));
    }
    va_end (strs);

    return str;
}



/*
    ">END"
*/
int
insert_output_end_marker (state_t *state)
{
    switch (state->debugger->index) {
    case (DEBUGGER_GDB):
        if (send_command (state,"echo >END\n") == FAIL)
            pfemr (ERR_DBG_CMD);
        break;
    case (DEBUGGER_PDB):
        if (send_command (state, "p \">END\"\n") == FAIL)
            pfemr (ERR_DBG_CMD);
        break;
    }
    return A_OK;
}

// TODO: Incorporate concatenate_strings() into send_command() functions

int
send_command_impl (state_t *state,
              int max_strs, ...)
{
    int str_count = 0;
    char *str;
    va_list strs;
    va_start (strs, max_strs);
    for (str = va_arg(strs, char*);
         str != NULL;
         str = va_arg(strs, char*))
    {
        if (write (state->debugger->stdin_pipe, str, strlen (str)) == -1)
            pfemr_errno ("Command write error for \"%s\"", str);
        if (++str_count >= max_strs)
            pfemr ("Max strings exceeded");
    }
    va_end (strs);
    return A_OK;
}



int
send_command_mp_impl (state_t *state,
                      int max_strs, ...)
{
    int str_count = 0;
    char *str;
    va_list strs;
    va_start (strs, max_strs);
    for (str = va_arg(strs, char*);
         str != NULL;
         str = va_arg(strs, char*))
    {
        if (write (state->debugger->stdin_pipe, str, strlen (str)) == -1)
            pfemr_errno ("Command write error for \"%s\"", str);
        if (++str_count >= max_strs)
            pfemr ("Max strings exceeded");
    }

    if (insert_output_end_marker (state) == FAIL)
        pfemr (ERR_OUT_MARK);

    if (parse_debugger_output (state) == FAIL)
        pfemr (ERR_DBG_PARSE);

    return A_OK;
}



/*
    Set Ncurses attribute with variable instead of constant
*/
int
set_nc_attribute (WINDOW* win,
                  int attr)
{
    switch (attr) {

        // colors (data.h)
        case RED_BLACK:
            wattron (win, COLOR_PAIR(RED_BLACK));
            break;
        case GREEN_BLACK:
            wattron (win, COLOR_PAIR(GREEN_BLACK));
            break;
        case YELLOW_BLACK:
            wattron (win, COLOR_PAIR(YELLOW_BLACK));
            break;
        case BLUE_BLACK:
            wattron (win, COLOR_PAIR(BLUE_BLACK));
            break;
        case MAGENTA_BLACK:
            wattron (win, COLOR_PAIR(MAGENTA_BLACK));
            break;
        case CYAN_BLACK:
            wattron (win, COLOR_PAIR(CYAN_BLACK));
            break;
        case WHITE_BLACK:
            wattron (win, COLOR_PAIR(WHITE_BLACK));
            break;

        // other
        case A_BOLD:
            wattron (win, A_BOLD);
            break;
        case A_UNDERLINE:
            wattron (win, A_UNDERLINE);
            break;

        default:
            pfemr ("Unsupported attribute \"%d\"", attr);
    }

    return A_OK;
}



/*
    Unset Ncurses attribute with variable instead of constant
*/
int
unset_nc_attribute (WINDOW* win,
                    int attr)
{
    switch (attr) {

        // colors (data.h)
        case RED_BLACK:
            wattroff (win, COLOR_PAIR(RED_BLACK));
            break;
        case GREEN_BLACK:
            wattroff (win, COLOR_PAIR(GREEN_BLACK));
            break;
        case YELLOW_BLACK:
            wattroff (win, COLOR_PAIR(YELLOW_BLACK));
            break;
        case BLUE_BLACK:
            wattroff (win, COLOR_PAIR(BLUE_BLACK));
            break;
        case MAGENTA_BLACK:
            wattroff (win, COLOR_PAIR(MAGENTA_BLACK));
            break;
        case CYAN_BLACK:
            wattroff (win, COLOR_PAIR(CYAN_BLACK));
            break;
        case WHITE_BLACK:
            wattroff (win, COLOR_PAIR(WHITE_BLACK));
            break;
        case WHITE_BLUE:
            wattroff (win, COLOR_PAIR(WHITE_BLUE));
            break;

        // other
        case A_BOLD:
            wattroff (win, A_BOLD);
            break;
        case A_UNDERLINE:
            wattroff (win, A_UNDERLINE);
            break;

        default:
            pfemr ("Unsupported attribute \"%d\"", attr);
    }

    return A_OK;
}



bool
find_window_string (WINDOW *window,
                    char   *string,
                    int    *y,
                    int    *x)
{
    int  i, j, 
         m, n,
         ch,
         rows, cols;
    size_t si = 0;
    bool found = false;

    // get number of rows, columns
    getmaxyx (window, rows, cols);

    // find string
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {

            ch = mvwinch (window, i, j);

            if ((char) ch == string [si]) {
                if (si == 0) {
                    m = i;
                    n = j;
                }
                si += 1;
                if (si == strlen (string)) {
                    found = true;
                    break;
                }
            } else {
                si = 0;
            }
        }
        if (found) {
            break;
        }
    }

    if (found) {
        *y = m;
        *x = n;
        return true;
    } else {
        return false;
    }
}



bool
file_was_updated (time_t file_mtime,
                  char *file_path)
{
    int ret;
    struct stat file_stat;

    ret = stat (file_path, &file_stat);
    if (ret == -1) {
        return false;
    }

    return file_mtime < file_stat.st_mtim.tv_sec;
}



void
set_state_ptr (state_t *state)
{
    state_ptr = state;
}



void
cp_wchar (buff_data_t *dest_buff_data,
          char ch)
{
    char *tmp;

    dest_buff_data->buff[dest_buff_data->buff_pos] = ch;
    dest_buff_data->buff[dest_buff_data->buff_pos + 1] = '\0';

    if (dest_buff_data->buff_pos < dest_buff_data->buff_len - 1) {
        ++dest_buff_data->buff_pos;
    } 

    else {

        // double buffer size
        if (dest_buff_data->times_doubled < MAX_DOUBLE_TIMES) {

            dest_buff_data->buff_len *= 2;
            ++dest_buff_data->times_doubled;

            // realloc
            if ((tmp = (char*) realloc (dest_buff_data->buff, sizeof (char) * dest_buff_data->buff_len)) == NULL)
                pfeme_errno ("Failed to reallocate window buffer (code: %s, times doubled: %d, buff size: %d)",
                             dest_buff_data->code, dest_buff_data->times_doubled, dest_buff_data->buff_len);

            dest_buff_data->buff = tmp;
        }

        // or loop back to buffer start
        else
            dest_buff_data->buff_pos = 0;
    }
}



void
cp_dchar (debugger_t *debugger,
          char        ch,
          int         buff_index)
{
    int  *len,
         *pos,
         *doubled;
    char *buff,
         *tmp,
         *title,
         *path_title  = "path",
         *form_title  = "format",
         *data_title  = "data",
         *cli_title   = "cli",
         *prog_title  = "program",
         *async_title = "async";


    switch (buff_index) {
        case PATH_BUF:
            title   =  path_title;
            buff    =  debugger->src_path_buffer;
            len     = &debugger->src_path_len;
            pos     = &debugger->src_path_pos;
            doubled = &debugger->src_path_times_doubled;
            break;
        case MAIN_PATH_BUF:
            title   =  path_title;
            buff    =  debugger->main_src_path_buffer;
            len     = &debugger->main_src_path_len;
            pos     = &debugger->main_src_path_pos;
            doubled = &debugger->main_src_path_times_doubled;
            break;
        case FORMAT_BUF:
            title   =  form_title;
            buff    =  debugger->format_buffer;
            len     = &debugger->format_len;
            pos     = &debugger->format_pos;
            doubled = &debugger->format_times_doubled;
            break;
        case DATA_BUF:
            title   =  data_title;
            buff    =  debugger->data_buffer;
            len     = &debugger->data_len;
            pos     = &debugger->data_pos;
            doubled = &debugger->data_times_doubled;
            break;
        case CLI_BUF:
            title   =  cli_title;
            buff    =  debugger->cli_buffer;
            len     = &debugger->cli_len;
            pos     = &debugger->cli_pos;
            doubled = &debugger->cli_times_doubled;
            break;
        case PROGRAM_BUF:
            title   =  prog_title;
            buff    =  debugger->program_buffer;
            len     = &debugger->program_len;
            pos     = &debugger->program_pos;
            doubled = &debugger->program_times_doubled;
            break;
        case ASYNC_BUF:
            title   =  async_title;
            buff    =  debugger->async_buffer;
            len     = &debugger->async_len;
            pos     = &debugger->async_pos;
            doubled = &debugger->async_times_doubled;
            break;
        default:
            pfeme ("Unrecognized debugger buffer index \"%d\"", buff_index);
    }

    buff [*pos] = ch;
    buff [*pos + 1] = '\0';

    if (*pos < *len - 1) {
        *pos += 1;
    } 

    else {

        // double buffer size
        if (*doubled < MAX_DOUBLE_TIMES) {

            *len     *= 2;
            *doubled += 1;

            if ((tmp = (char*) realloc (buff, sizeof (char) * *len)) == NULL)
                pfeme_errno  ("Failed to reallocate \"%s\" buffer (size: %d, position: %d, times doubled: %d)", title, *len, *pos, *doubled);

            switch (buff_index) {
                case PATH_BUF:
                    debugger->src_path_buffer = tmp;
                    break;
                case FORMAT_BUF:
                    debugger->format_buffer = tmp;
                    break;
                case DATA_BUF:
                    debugger->data_buffer = tmp;
                    break;
                case CLI_BUF:
                    debugger->cli_buffer = tmp;
                    break;
                case PROGRAM_BUF:
                    debugger->program_buffer = tmp;
                    break;
                case ASYNC_BUF:
                    debugger->async_buffer = tmp;
                    break;
            }
        }

        // or loop back to buffer start
        else
            *pos = 0;
    }
}



int
copy_to_clipboard (char *str)
{
    char *cmd_str;

    if ((cmd_str = concatenate_strings ("printf \"", str, "\" | xclip -selection clipboard")) == NULL)
        pfemr ("Failed to create string \"%s\"", str);

    if (system (cmd_str) == -1)
        pfemr_errno ("system() error: \"%s\"", strerror (errno));

    free (cmd_str);

    return A_OK;
}



char*
create_buff_from_file (char *path)
{
    int ch, i;
    struct stat st;
    FILE *fp;
    char *buff;

    // create buffer
    if (stat (path, &st) != 0)
        pfemn_errno  ("Failed to get status of file \"%s\"", path);
    if ((buff = (char*) malloc (st.st_size + 1)) == NULL)
        pfemn_errno  ("Failed to allocate buffer for path \"%s\"", path);

    // copy file contents
    if ((fp = fopen (path, "r")) == NULL)
        pfemn_errno  ("Failed to open file \"%s\"", path);
    i = 0;
    while ((ch = fgetc (fp)) != EOF && i < st.st_size)
        buff [i++] = ch;
    buff [i] = '\0';
    fclose (fp);

    return buff;
}



