#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>

#include "utilities.h"
#include "error.h"
#include "plugins.h"
#include "data.h"
#include "debugger.h"
#include "persist_data.h"

FILE *debug_out_ptr = NULL;
bool program_cleaned_up = false;
state_t *state_ptr = NULL;



void
logd (const char *formatted_string, ...)
{
    if (debug_out_ptr == NULL) {
        if ((debug_out_ptr = fopen (DEBUG_OUT_FILE, "w")) == NULL) {
            pfeme_errno ("Failed to open debug out file \"%s\"", DEBUG_OUT_FILE);
        }
    }

    va_list args;
    va_start (args, formatted_string);
    vfprintf (debug_out_ptr, formatted_string, args);
    va_end (args);
}



void clean_up (int type)
{
    if (program_cleaned_up == false) {
        program_cleaned_up = true;

        //TODO: clean_up_debugger (debugger);

        if (state_ptr) {

            if (state_ptr->header != NULL)
                if (delwin (state_ptr->header) == ERR)
                    pfem ("Failed to delete ncurses header subwindow");

            if (free_nc_window_data (state_ptr) == FAIL)
                pfem (ERR_NC_FREE);

            curs_set (1);
            endwin ();

            if (persist_data (state_ptr) == FAIL)
                pfem (ERR_PERSIST);
        }

        if (debug_out_ptr)
            if (fclose (debug_out_ptr) != 0)
                pfem_errno (ERR_DBG_FCLOSE);

        if (type == PROG_ERROR)
            fprintf (stderr,  RED "ERROR" CYAN " :: " R "termfu exited\n");
    }
}

/*
    SIGINT handler for Ctrl-C
*/
static void
sigint_handler (int sig_num)
{
    (void) sig_num;
    clean_up (PROG_EXIT);
    fprintf (stderr, "termfu exited (SIGINT)\n");
    exit (EXIT_FAILURE);
}



int
initial_configure (int   argc,
                   char *argv[],
                   state_t *state)
{
    int opt;
    extern char *optarg;

    state->config_path[0] = '\0';
    state->data_path[0]   = '\0';

    char *optstring = "hc:p:";

    while ((opt = getopt (argc, argv, optstring)) != -1) {
        switch (opt) {

            // help
            case 'h':
                printf (
                "\n"
                "Usage: \n"
                "\n"
                "   $ termfu\n"
                "\n"
                "       Run in same directory as a %s configuration file\n"
                "       Data persisted to ./%s\n"
                "\n"
                "   $ termfu [OPTION...]\n"
                "\n"
                "       -c CONFIG_FILE    Use this configuration file\n"
                "       -p PERSIST_FILE   Persist sessions with this file\n"
                "\n",
                CONFIG_FILE, PERSIST_FILE);
                exit (EXIT_SUCCESS);

            // configuration file
            case 'c':
                strncpy (state->config_path, optarg, CONFIG_PATH_LEN - 1);
                break;

            // data persist file
            case 'p':
                strncpy (state->data_path, optarg, DATA_PATH_LEN - 1);
                break;

            default:
                fprintf (stderr,
                "\n"
                "Run with -h flag to see usage instructions.\n"
                "\n");
                exit (EXIT_FAILURE);
        }
    }

    state->new_run = true;
    state->restart_prog = false;
    state_ptr = state;

    signal (SIGINT, sigint_handler);     // Ctrl-C; (gdb) signal 2

    // ncurses
    initscr ();
    if (has_colors ()) {
        start_color();
        init_pair(RED_BLACK, COLOR_RED, COLOR_BLACK);           // RED_BLACK
        init_pair(GREEN_BLACK, COLOR_GREEN, COLOR_BLACK);       // GREEN_BLACK
        init_pair(YELLOW_BLACK, COLOR_YELLOW, COLOR_BLACK);     // YELLOW_BLACK
        init_pair(BLUE_BLACK, COLOR_BLUE, COLOR_BLACK);         // BLUE_BLACK
        init_pair(MAGENTA_BLACK, COLOR_MAGENTA, COLOR_BLACK);   // MAGENTA_BLACK
        init_pair(CYAN_BLACK, COLOR_CYAN, COLOR_BLACK);         // CYAN_BLACK
        init_pair(WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);       // WHITE_BLACK
        init_pair(WHITE_BLUE, COLOR_WHITE, COLOR_BLUE);         // WHITE_BLUE
        init_pair(BLACK_BLUE, COLOR_BLACK, COLOR_BLUE);         // WHITE_BLUE
    } 
    cbreak ();
    noecho ();
    curs_set (0);
    set_escdelay (0);
    keypad (stdscr, TRUE);

    return A_OK;
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

char*
create_buff_from_file (char *path)
{
    int ch, i;
    struct stat st;
    FILE *fp;
    char *buff;

    // Create buffer
    if (stat (path, &st) != 0)
        pfemn_errno  ("Failed to get status of file \"%s\"", path);
    if ((buff = (char*) malloc (st.st_size + 1)) == NULL)
        pfemn_errno  ("Failed to allocate buffer for path \"%s\"", path);

    if ((fp = fopen (path, "r")) == NULL)
        pfemn_errno  ("Failed to open file \"%s\"", path);

    // Copy file contents
    i = 0;
    while ((ch = fgetc (fp)) != EOF && i < st.st_size)
        buff [i++] = ch;
    buff [i] = '\0';

    fclose (fp);

    return buff;
}



#ifdef DEV

#define MAX_CMD_STRS 40

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



void log_state_t (state_t *state)
{
    int i;
    layout_t *curr_layout = NULL;
    breakpoint_t *curr_break = NULL;
    watchpoint_t *curr_watch = NULL;

    logd ("state->\n"
          "\tnum_plugins: %d\n"
          "\tplugin_key_index:\n", state->num_plugins);
    if (state->plugin_key_index != NULL)
        for (i = 0; i < state->num_plugins; i++)
                logd ("\t\t[%d] %d\n", i, state->plugin_key_index[i]);
    else
        logd ("\t\tNULL\n");

    logd ("\tconfig_path: \"%s\"\n"
            "\tdata_path: \"%s\"\n"
            "\tinput_buff: \n\n\"%s\"\n\n",
            state->config_path,
            state->data_path,
            state->input_buffer);

    logd ("\tplugins:\n");
    if (state->plugins != NULL)
        for (i = 0; i < state->num_plugins; i++)
            logd ("\t\t[%d] \"%s\"\n", i, state->plugins[i]->code);
    else
        logd ("NULL\n");

    logd ("\tlayouts:\n");
    curr_layout = state->layouts;
    if (state->layouts != NULL)
        do {
            logd ("\t\t\"%s\"\n", curr_layout->label);
            curr_layout = curr_layout->next;
        } while (curr_layout != NULL);
    else
        printf ("\t\tNULL\n");

    logd ("\tcurr_layout: \"%s\"\n"
          "\theader: %p\n"
          "\n"
          "\tdebugger: %p\n",
          state->curr_layout,
          state->header,
          state->debugger);

    logd ("\tcommand:\n");
    if (state->command != NULL) {
        logd ("\t\t");
        for (i = 0; i < MAX_CMD_STRS; i++) {
            if (state->command[i] != NULL) {
                logd ("%s ", state->command[i]);
            } else {
                break;
            }
        }
        logd ("\n");
    } else {
        logd ("\t\tNULL\n");
    }

    logd ("\twatchpoints:\n");
    curr_watch = state->watchpoints;
    if (curr_watch != NULL) {
        while (curr_watch != NULL) {
            logd ("\t\t\"%s\" \"%s\"\n", curr_watch->var, curr_watch->value);
            curr_watch = curr_watch->next;
        }
    } else {
        logd ("\t\tNULL\n");
    }

    logd ("\tbreakpoints:\n");
    curr_break = state->breakpoints;
    if (curr_break != NULL) {
        while (curr_break != NULL) {
            logd ("\t\t\"%s\" \"%s\"\n", curr_break->path, curr_break->line);
            curr_break = curr_break->next;
        }
    } else {
        logd ("\t\tNULL\n");
    }

    logd ("\tnew_run: %d\n", state->new_run);
}

#endif

