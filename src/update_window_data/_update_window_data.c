
/*
    Send window data update commands
*/
#include <stdarg.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../display_lines.h"

#include "get_breakpoint_data.h"

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

    switch (plugin_index) {
    case Asm:
        break;
    case Brk: 
        get_breakpoint_data (state); 
        display_lines (BUFF_TYPE, NEW_WIN, win);
        break;
    case LcV:
        break;
    case Reg:
        break;
    case Src:
        break;
    case Wat:
        break;
    }
}



