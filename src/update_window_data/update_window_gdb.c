
#include "update_window_gdb.h"
#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../utilities.h"
#include "../display_lines.h"
#include "../insert_output_marker.h"
#include "../parse_debugger_output.h"
#include "../get_source_file.h"



void
update_assembly_gdb (state_t *state)
{
    window_t *win = state->plugins[Asm]->win;

    insert_output_start_marker (state);
    send_command (state, "x/i $pc\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    copy_string_buffer (state->debugger->cli_buffer, win->buff_data->buff);
    get_buff_rows_cols (win);

    display_lines (BUFF_TYPE, NEW_WIN, win);
}



void
update_breakpoints_gdb (state_t *state)
{
    window_t *win = state->plugins[Brk]->win;

    insert_output_start_marker (state);
    send_command (state, "info break\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    copy_string_buffer (state->debugger->cli_buffer, win->buff_data->buff);
    get_buff_rows_cols (win);

    display_lines (BUFF_TYPE, NEW_WIN, win);
}



void
update_local_vars_gdb (state_t *state)
{
    window_t *win = state->plugins[LcV]->win;

    insert_output_start_marker (state);
    send_command (state, "info locals\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    copy_string_buffer (state->debugger->cli_buffer, win->buff_data->buff);
    get_buff_rows_cols (win);

    display_lines (BUFF_TYPE, NEW_WIN, win);
}



void
update_registers_gdb (state_t *state)
{
    window_t *win = state->plugins[Reg]->win;

    insert_output_start_marker (state);
    send_command (state, "info registers\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    copy_string_buffer (state->debugger->cli_buffer, win->buff_data->buff);
    get_buff_rows_cols (win);

    display_lines (BUFF_TYPE, NEW_WIN, win);
}



void
update_source_file_gdb (state_t *state)
{
    window_t *win = state->plugins[Src]->win;

    get_source_file_path_and_line_num (state);

    //display_lines (FILE_TYPE, NEW_WIN, win);
}



void
update_watchpoints_gdb (state_t *state)
{
    window_t *win = state->plugins[Wat]->win;

    insert_output_start_marker (state);
    send_command (state, "info watchpoints\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    copy_string_buffer (state->debugger->cli_buffer, win->buff_data->buff);
    get_buff_rows_cols (win);

    display_lines (BUFF_TYPE, NEW_WIN, win);
}



