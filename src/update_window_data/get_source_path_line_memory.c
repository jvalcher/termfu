
#include <ncurses.h>

#include "get_source_path_line_memory.h"
#include "../data.h"
#include "../plugins.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"

void  get_source_path_line_memory_gdb  (state_t *state);



void
get_source_path_line_memory (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_source_path_line_memory_gdb (state);
            break;
    }
}



void
get_source_path_line_memory_gdb (state_t *state)
{
    char *src_ptr,
         *dest_ptr,
         *prev_ptr;
    window_t *win;
    bool is_running;
    file_data_t *file_data;

    const char *key_threads  = "threads=[",
               *key_addr     = "addr=\"",
               *key_func     = "func=\"",
               *key_fullname = "fullname=\"",
               *key_line     = "line=\"";

    win = state->plugins[Src]->win;
    src_ptr = state->debugger->data_buffer;
    file_data = win->file_data;
    is_running = true;

    file_data->path_pos = 0;
    file_data->addr_pos = 0;
    file_data->func_pos = 0;

    // check if program running
    insert_output_start_marker (state);
    send_command (state, "-thread-info\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

        // get number of threads
    src_ptr = strstr (src_ptr, key_threads);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_threads);
    } else {
        *src_ptr = ']';
    }


    // if no threads or program not running
    if (*src_ptr == ']') {

        insert_output_start_marker (state);
        send_command (state, "-file-list-exec-source-files\n");
        insert_output_end_marker (state);
        parse_debugger_output (state);

        src_ptr = state->debugger->data_buffer;
        is_running = false;
    }

    // memory address
    file_data->addr_pos = 0;
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_addr);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_addr);
        while (*src_ptr != '\"') {
            cp_fchar (file_data, *src_ptr++, ADDR);
        }
    } else {
        src_ptr = prev_ptr;
    }
    cp_fchar (file_data, '\0', ADDR);

    // function
    file_data->func_pos = 0;
    prev_ptr = src_ptr;
    src_ptr = strstr (src_ptr, key_func);
    if (src_ptr != NULL) {
        src_ptr += strlen (key_func);
        while (*src_ptr != '\"') {
            cp_fchar (file_data, *src_ptr++, FUNC);
        }
    } else {
        src_ptr = prev_ptr;
    }
    cp_fchar (file_data, '\0', FUNC);
    
    // absolute path
    src_ptr = strstr (src_ptr, key_fullname);
    dest_ptr = state->debugger->format_buffer;
    if (src_ptr != NULL) {
        src_ptr += strlen (key_fullname);
        while (*src_ptr != '\"') {
            *dest_ptr++ = *src_ptr++;
        }
        *dest_ptr = '\0';

        if (strcmp (state->debugger->format_buffer, win->file_data->path) != 0) {
            strncpy (win->file_data->path, state->debugger->format_buffer, FILE_PATH_LEN - 1);
            win->file_data->path_changed = true;
        }
        // TODO: else if { file timestamp changed, path_changed = true }

        // line number
        dest_ptr = state->debugger->format_buffer;
        if (is_running) {
            src_ptr = strstr (src_ptr, key_line);
            src_ptr += strlen (key_line);
            while (*src_ptr != '\"') {
                *dest_ptr++ = *src_ptr++;
            }
            *dest_ptr = '\0';
            win->file_data->line = atoi (state->debugger->format_buffer);
        } else {
            win->file_data->line = 0;
        }
    }
}
