
#include <ncurses.h>

#include "get_source_file.h"
#include "data.h"
#include "plugins.h"
#include "insert_output_marker.h"
#include "parse_debugger_output.h"
#include "utilities.h"

void  get_source_file_gdb  (state_t *state);



void
get_source_file_path_and_line_num (state_t *state)
{
    switch (state->debugger->curr) {
        case (DEBUGGER_GDB):
            get_source_file_gdb (state);
            break;
    }
}



void
get_source_file_gdb (state_t *state)
{
    char *src_ptr,
         *dest_ptr;
    window_t *win;
    bool is_running;

    const char *key_threads  = "threads=[",
               *key_fullname = "fullname=\"";

    win = state->plugins[Src]->win;
    src_ptr = state->debugger->data_buffer;
    dest_ptr = state->debugger->format_buffer;

    // check if running program
        // send command
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
    }

    // get absolute path
    src_ptr = strstr (src_ptr, key_fullname);
    src_ptr += strlen (key_fullname);
    while (*src_ptr != '\"') {
        *dest_ptr++ = *src_ptr++;
    }
    *dest_ptr = '\0';
}
