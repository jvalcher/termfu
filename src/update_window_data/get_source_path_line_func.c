#include <ncurses.h>

#include "get_source_path_line_func.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"

static int get_source_path_line_func_gdb (state_t *state);
static int get_source_path_line_func_pdb (state_t *state);

bool first_run = true;



int
get_source_path_line_func (state_t *state)
{
    int ret;

    switch (state->debugger->index) {
        case (DEBUGGER_GDB):
            ret = get_source_path_line_func_gdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get source, line, memory (GDB)");
            }
            break;
        case (DEBUGGER_PDB):
            ret = get_source_path_line_func_pdb (state);
            if (ret == FAIL) {
                pfemr ("Failed to get source, line, memory (PDB)");
            }
            break;
    }

    return A_OK;
}



static int
get_source_path_line_func_gdb (state_t *state)
{
    int   ret, i,
          curr_index;
    char *src_ptr,
         *path_ptr,
         *main_path_ptr = NULL,
         *prev_ptr;
    window_t *win;
    bool is_running;
    debugger_t  *debugger;

    const char *key_threads  = "threads=[",
               *key_fullname = "fullname=\"",
               *key_func     = "func=\"",
               *key_line     = "line=\"";

    win = state->plugins[Src]->win;
    debugger = state->debugger;
    curr_index = debugger->curr_plugin_index;

    debugger->src_path_pos = 0;
    debugger->src_path_buffer[0] = '\0';

    // check if program running
    src_ptr = state->debugger->data_buffer;
    ret = send_command_mp (state, "-thread-info\n");
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }

        // get number of threads
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_threads);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_threads);
    } else {
        src_ptr = prev_ptr;
        *src_ptr = ']';
    }


    // if no threads or program not running
    if (*src_ptr == ']') {
        ret = send_command_mp (state, "-file-list-exec-source-files\n");
        if (ret == FAIL) {
            pfemr (ERR_DBG_CMD);
        }
        src_ptr = state->debugger->data_buffer;
        is_running = false;
    } else {
        is_running = true;
    }

    // function
    i = 0;
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_func);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_func);
        while (*src_ptr != '\"' && i < FUNC_LEN) {
            debugger->curr_func [i++] = *src_ptr++;
        }
    } else {
        src_ptr = prev_ptr;
    }
    debugger->curr_func [i] = '\0';

    // absolute path
    debugger->format_pos = 0;
    debugger->format_buffer[0] = '\0';

    src_ptr = strstr (src_ptr, key_fullname);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_fullname);
        while (*src_ptr != '\"') {
            cp_dchar (debugger, *src_ptr++, FORMAT_BUF);
        }

        // path changed
        if (strcmp (debugger->format_buffer, debugger->src_path_buffer) != 0) {

            // set buffer
            switch (curr_index) {
                case Kil:
                    path_ptr = debugger->main_src_path_buffer;
                    break;
                default:
                    if (is_running || first_run) {
                        path_ptr = debugger->format_buffer;
                        main_path_ptr = path_ptr;
                    } else {
                        path_ptr = debugger->main_src_path_buffer;
                    }
            }

            // copy to debugger->src_path_buffer
            debugger->src_path_pos = 0;
            while (*path_ptr != '\0') {
                cp_dchar (debugger, *path_ptr++, PATH_BUF);
            }

            // copy to debugger->main_src_path_buffer
            if (first_run) {
                while (*main_path_ptr != '\0') {
                    cp_dchar (debugger, *main_path_ptr++, MAIN_PATH_BUF);
                }
                first_run = false;
            }

            debugger->src_path_changed = true;
        } 

        // line number
        debugger->format_pos = 0;
        if (is_running) {
            src_ptr = strstr (src_ptr, key_line);
            src_ptr += strlen (key_line);
            while (*src_ptr != '\"') {
                cp_dchar (debugger, *src_ptr++, FORMAT_BUF);
            }
            debugger->curr_Src_line = atoi (debugger->format_buffer);
        } else {
            debugger->curr_Src_line = 1;
        }

        win->buff_data->changed = true;
    }

    return A_OK;
}



static int
get_source_path_line_func_pdb (state_t *state)
{
    int   ret;
    char *src_ptr,
         *path_ptr;
    debugger_t  *debugger;
    buff_data_t *buff_data;

    const char *curr_path_symbol = "\n> ",
               *caret_str        = "<string>";

    debugger  = state->debugger;
    src_ptr   = state->debugger->cli_buffer;
    buff_data = state->plugins[Src]->win->buff_data;

    debugger->src_path_pos = 0;
    debugger->src_path_buffer[0] = '\0';

    ret = send_command_mp (state, "where\n");
    if (ret == FAIL) {
        pfemr (ERR_DBG_CMD);
    }

    src_ptr = strstr (src_ptr, curr_path_symbol);
    src_ptr += strlen (curr_path_symbol);

    // path
    state->debugger->format_pos = 0;
    state->debugger->format_buffer[0] = '\0';

    while (*src_ptr != '(') {
        cp_dchar (debugger, *src_ptr++, FORMAT_BUF);
    }
    if (strcmp (debugger->format_buffer, caret_str) != 0) {
        if (strcmp (debugger->format_buffer, debugger->src_path_buffer) != 0) {
            path_ptr = debugger->format_buffer;
            debugger->src_path_pos = 0;
            while (*path_ptr != '\0') {
                cp_dchar (debugger, *path_ptr++, PATH_BUF);
            }
            buff_data->changed = true;
            debugger->src_path_changed = true;
        }

        ++src_ptr;

        // line number
        debugger->format_pos = 0;
        do {
            cp_dchar (debugger, *src_ptr++, FORMAT_BUF);
        } while (*src_ptr != ')');
        debugger->curr_Src_line = atoi (debugger->format_buffer);
    }

    return A_OK;
}
