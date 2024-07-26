
#include "update_window_gdb.h"
#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"



void
update_asm_gdb (state_t *state)
{
    window_t *win = state->plugins[Asm]->win;

    send_command (state, 2, "x/i ", "$pc\n");

    copy_string_buffer (state->debugger->debugger_buffer, win->buff_data->buff);

    get_buff_rows_cols (win);
}
