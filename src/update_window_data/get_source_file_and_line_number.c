
#include <ncurses.h>

#include "get_source_file_and_line_number.h"
#include "../data.h"
#include "../plugins.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../utilities.h"

void  get_source_path_line_gdb  (state_t *state);



void
get_source_file_path_and_line_number (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_source_path_line_gdb (state);
            break;
    }
}



void
get_source_path_line_gdb (state_t *state)
{
    char *src_ptr,
         *dest_ptr;
    window_t *win;
    bool is_running;

    const char *key_threads  = "threads=[",
               *key_fullname = "fullname=\"",
               *key_line     = "line=\"";

    win = state->plugins[Src]->win;
    src_ptr = state->debugger->data_buffer;
    is_running = true;

    // check if program running
    insert_output_start_marker (state);
    send_command (state, "-thread-info\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

        // get number of threads
    src_ptr = strstr (src_ptr, key_threads);
    src_ptr += strlen (key_threads);

    // if program not running
    if (*src_ptr == ']') {

        insert_output_start_marker (state);
        send_command (state, "-file-list-exec-source-files\n");
        insert_output_end_marker (state);
        parse_debugger_output (state);

        src_ptr = state->debugger->data_buffer;
        is_running = false;
    }

    // get absolute path
    dest_ptr = state->debugger->format_buffer;
    src_ptr = strstr (src_ptr, key_fullname);
    src_ptr += strlen (key_fullname);
    while (*src_ptr != '\"') {
        *dest_ptr++ = *src_ptr++;
    }
    *dest_ptr = '\0';

    // set absolute path if changed
    if (strcmp (state->debugger->format_buffer, win->file_data->path) != 0) {
        strncpy (win->file_data->path, state->debugger->format_buffer, FILE_PATH_LEN - 1);
        win->file_data->path_changed = true;
    }

    // set line number
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
