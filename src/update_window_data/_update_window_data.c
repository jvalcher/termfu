
/*
    Send window data update commands
*/
#include <stdarg.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../display_lines.h"

#include "get_assembly_data.h"
#include "get_breakpoint_data.h"
#include "get_debugger_output.h"
#include "get_source_file_and_line_number.h"

void  update_window  (int, state_t*);



void
update_windows (state_t *state,
                int num_updates,
                ...)
{
    int plugin;
    va_list  plugins;

    va_start (plugins, num_updates);

    for (int i = 0; i < num_updates; i++) {
        plugin = va_arg (plugins, int);
        update_window (plugin, state);
    }

    va_end (plugins);
}



void
update_window (int      plugin_index,
               state_t *state)
{
    window_t *win = state->plugins[plugin_index]->win;
    int buff_type = win->has_data_buff ? BUFF_TYPE : FILE_TYPE;
    int data_position = BEG_DATA;

    switch (plugin_index) {
    case Asm:
        get_assembly_data (state);
        break;
    case Brk: 
        get_breakpoint_data (state); 
        break;
    case Dbg:
        get_debugger_output (state);
        data_position = END_DATA;
        break;
    case LcV:
        break;
    case Prg:
        data_position = END_DATA;
        break;
    case Reg:
        break;
    case Src:
        get_source_file_path_and_line_number (state);
        data_position = LINE_DATA;
        break;
    case Wat:
        break;
    }

    display_lines (buff_type, data_position, win);
}



