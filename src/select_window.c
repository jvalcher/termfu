
#include "select_window.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "render_window.h"
#include "insert_output_marker.h"
#include "update_windows.h"
#include "parse_debugger_output.h"

#define ESC  27



void
select_window (int      plugin_index,
               state_t *state)
{
    int       key,
              curr_debugger;
    bool      in_loop         = true,
              key_not_pressed = true;

    curr_debugger = state->debugger->curr;

    render_window (SELECT, -1, plugin_index, state);

    while (in_loop) {

        key = getkey();

        // TODO: pg up/down, home, end
        switch (key) {
        case KEY_UP:
            render_window (DATA, KEY_UP, plugin_index, state);
            key_not_pressed = false;
            break;
        case KEY_DOWN:
            render_window (DATA, KEY_DOWN, plugin_index, state);
            key_not_pressed = false;
            break;
        case KEY_RIGHT:
            render_window (DATA, KEY_RIGHT, plugin_index, state);
            key_not_pressed = false;
            break;
        case KEY_LEFT:
            render_window (DATA, KEY_LEFT, plugin_index, state);
            key_not_pressed = false;
            break;
        case ESC:
            in_loop = false;
            key_not_pressed = false;
        }

        // custom keys
        if (in_loop && key_not_pressed) {
            if      (key == state->plugins[Bak]->key) {
                in_loop = false;
                continue;
            } 
            else if (key == state->plugins[Qut]->key) {
                in_loop                  = false;
                state->debugger->running = false;
                continue;
            }
            else if (key == state->plugins[ScU]->key) {
                render_window (DATA, KEY_UP, plugin_index, state);
                continue;
            } 
            else if (key == state->plugins[ScD]->key) {
                render_window (DATA, KEY_DOWN, plugin_index, state);
                continue;
            } 
            else if (key == state->plugins[ScL]->key) {
                render_window (DATA, KEY_LEFT, plugin_index, state);
                continue;
            } 
            else if (key == state->plugins[ScR]->key) {
                render_window (DATA, KEY_RIGHT, plugin_index, state);
                continue;
            }

            switch (plugin_index) {
            case Brk:
                switch (curr_debugger) {
                case (DEBUGGER_GDB): 
                    insert_output_start_marker (state);
                    send_command (state, 2, "-break-insert ", " main\n"); 
                    insert_output_end_marker (state);
                    parse_debugger_output (state);
                    update_windows (state, 1, Brk);
                    break;   // TODO: breaks
            }
            break;
            }
        }

        key_not_pressed = true;
    }

    render_window (DESELECT, -1, plugin_index, state);
}
