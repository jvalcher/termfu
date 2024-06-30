
#include "select_window.h"
#include "data.h"
#include "utilities.h"
#include "plugins.h"
#include "render_window_data.h"

#define ESC  27



void
select_window (int      plugin_index,
               state_t *state)
{
    int       key;
    bool      in_loop         = true,
              key_not_pressed = true;

    render_window_data (plugin_index, state, KEY_UP, SELECT_WINDOW_COLOR);

    while (in_loop) {

        key = getkey();

        // TODO: pg up/down, home, end
        switch (key) {
        case KEY_UP:
            render_window_data (plugin_index, state, KEY_UP, RENDER_DATA);
            key_not_pressed = false;
            break;
        case KEY_DOWN:
            render_window_data (plugin_index, state, KEY_DOWN, RENDER_DATA);
            key_not_pressed = false;
            break;
        case KEY_RIGHT:
            render_window_data (plugin_index, state, KEY_RIGHT, RENDER_DATA);
            key_not_pressed = false;
            break;
        case KEY_LEFT:
            render_window_data (plugin_index, state, KEY_LEFT, RENDER_DATA);
            key_not_pressed = false;
            break;
        case ESC:
            in_loop = false;
        }

        // TODO: implement plugin specific keys
        switch (plugin_index) {}

        // custom keys
        if (in_loop && key_not_pressed) {
            if      (key == state->plugins[Bak]->key) {
                in_loop = false;
            } 
            else if (key == state->plugins[Qut]->key) {
                in_loop                  = false;
                state->debugger->running = false;
            }
            else if (key == state->plugins[ScU]->key) {
                render_window_data (plugin_index, state, KEY_UP, RENDER_DATA);
            } 
            else if (key == state->plugins[ScD]->key) {
                render_window_data (plugin_index, state, KEY_DOWN, RENDER_DATA);
            } 
            else if (key == state->plugins[ScL]->key) {
                render_window_data (plugin_index, state, KEY_LEFT, RENDER_DATA);
            } 
            else if (key == state->plugins[ScR]->key) {
                render_window_data (plugin_index, state, KEY_RIGHT, RENDER_DATA);
            }
        }

        key_not_pressed = true;
    }

    render_window_data (plugin_index, NULL, key, DESELECT_WINDOW_COLOR);
}
