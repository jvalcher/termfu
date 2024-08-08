
#include <ncurses.h>

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
        case (DEBUGGER_GDB): get_source_file_gdb (state); break;
    }
}



void
get_source_file_gdb (state_t *state)
{
    window_t *win = state->plugins[Src]->win;

    insert_output_start_marker (state);
    send_command (state, "-file-list-exec-source_file\n");
    insert_output_end_marker (state);

    parse_debugger_output (state);

    mvwprintw (win->DWIN, 0, 0, "%s", state->debugger->data_buffer);
}
