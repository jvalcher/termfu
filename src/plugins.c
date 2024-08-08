/*

Plugins
------
- Each case-sensitive, three-letter code is associated with a plugin
- Each plugin is assigned a key shortcut [A-Z,a-z] in the configuration file

- Codes:

    - Window

        Asm     Assembly code
        Brk     Breakpoints
        LcV     Local variables
        Out     Program output
        Prm     Debugger prompt, output
        Reg     Registers
        Src     Source file
        Wat     Watchpoints

    - Pop-up window

        Bld     Project build selection
        Lay     Layout selection

    - Non-window

        EMP     Empty function
        Bak     Back out of window
        Qut     Quit

        Con     Continue
        Fin     Finish
        Kil     Kill
        Nxt     Next
        Run     (Re)run program
        Stp     Step
        Unt     Until

    - Misc
        ScU     Scroll window up
        ScD     Scroll window down
        ScL     Scroll window left
        ScR     Scroll window right
*/

#include <string.h>
#include <time.h>

#include "plugins.h"
#include "data.h"
#include "utilities.h"


/*
    Plugin codes
    -----------
    - Indexes match corresponding indexes in:
        - plugin_function []
        - win_file_path []

    - Used in bind_keys_windows_to_plugins()
        - Ordered alphabetically for binary search  (A-Z,a-z)
*/
char *plugin_codes [] = {
    
    "EMP",
    "Asm",
    "Bak",
    "Bld",
    "Brk",
    "Con",
    "Fin",
    "Kil",
    "Lay",
    "LcV",
    "Nxt",
    "Out",
    "Prm",
    "Qut",
    "Reg",
    "Run",
    "ScD",
    "ScL",
    "ScR",
    "ScU",
    "Src",
    "Stp",
    "Unt",
    "Wat"
};



int win_plugins[]      = { Asm, Brk, LcV, Out, Prm, Reg, Src, Wat };
int win_buff_plugins[] = { Asm, Brk, LcV, Out, Prm, Reg, Src, Wat };    // TODO: Src -> file type
int win_file_plugins[] = { Src };

int win_input_plugins[] = {
    Brk,
    Src,
    Wat
};
char *input_titles[] = {
    " (d)elete",
    " (i)nsert break ",
    " (c)reate  (d)elete "
};



void
set_window_plugins (state_t *state)
{
    int i, j,
        num_win_plugins       = sizeof (win_plugins) / sizeof (win_plugins[0]),
        num_win_input_plugins = sizeof (win_input_plugins) / sizeof (win_input_plugins[0]),
        // TODO: num_win_file_plugins  = sizeof (win_file_plugins) / sizeof (win_file_plugins[0]),
        num_win_file_plugins = 0,
        num_win_buff_plugins  = sizeof (win_buff_plugins) / sizeof (win_buff_plugins[0]);
    window_t *win;
    plugin_t *plugin;

    for (i = 0; i < state->num_plugins; i++) {
        state->plugins[i]->has_window = false;
    }

    // allocate window_t structs
    for (i = 0; i < num_win_plugins; i++) {

        j = win_plugins[i];
        plugin = state->plugins[j];
        win = plugin->win;
        win = (window_t*) malloc (sizeof (window_t));
        if (win == NULL) {
            pfeme ("Window malloc failed for plugin %s (code: %s, index: %d)",
                    plugin->title, plugin->code, i);
        }
        win->has_input = false;
    }

    // set input windows
    for (i = 0; i < num_win_input_plugins; i++) {
        j = win_input_plugins[i];
        win = state->plugins[j]->win;
            //
        win->has_input = true;
        win->input_title = input_titles[i];
    }

    // allocate file data
    for (i = 0; i < num_win_file_plugins; i++) {
        j = win_file_plugins[i];
        win = state->plugins[j]->win;
            //
        win->has_data_buff = false;
        win->file_data = (file_data_t*) malloc (sizeof (file_data_t));
        win->file_data->path_changed = true;
        win->buff_data = NULL;
    }

    // allocate buffer data
    for (i = 0; i < num_win_buff_plugins; i++) {
        j = win_buff_plugins[i];
        win = state->plugins[j]->win;
            //
        win->has_data_buff = true;
        win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
        win->buff_data->scroll_col = 1;
        win->buff_data->scroll_row = 1;
        win->file_data = NULL;
    }
}



void
set_num_plugins (state_t *state)
{
    state->num_plugins = sizeof (plugin_codes) / sizeof (plugin_codes [0]);

}



int
get_plugin_code_index (char    *code,
                       state_t *state)
{
    int si = 1,
        mi,
        ei = state->num_plugins - 1, 
        r;
    while (si <= ei) {
        mi = si + (ei - si) / 2;
        r = strcmp (plugin_codes [mi], code);
        if (r == 0) {
            return mi;
        } else if (r < 0) {
            si = mi + 1;
        } else {
            ei = mi - 1;
        }
    }
    return -1;
}



