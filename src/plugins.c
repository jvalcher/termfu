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



int win_plugins[] = { Asm, Brk, LcV, Out, Prm, Reg, Src, Wat };

int win_file_data[] = { Src };   // as opposed to buffer



int win_input_plugins[] = {
    Brk,
    Prm,
    Src,
    Wat
};
char *input_titles[] = {
    " (d)elete",
    " (c)ommand",
    " Create (b)reak ",
    " (c)reate  (d)elete "
};
char *input_prompts[] = {
    " Break num: ",
    " Command: ",
    " Select line...",
    " Watch: "
};



void
set_window_plugins (state_t *state)
{
    int num_win_plugins = sizeof (win_plugins) / sizeof (win_plugins[0]);
    int num_win_input_plugins = sizeof (win_input_plugins) / sizeof (win_input_plugins[0]);
    int num_win_file_plugins = sizeof (win_file_data) / sizeof (win_file_data[0]);

    for (int i = 0; i < state->num_plugins; i++) {

        for (int j = 0; j < num_win_plugins; j++) {

            // if window...
            if (i == win_plugins[j]) {

                state->plugins[i]->has_window = true;

                // input window
                for (int k = 0; k < num_win_input_plugins; k++) {
                    if (i == win_input_plugins[k]) {
                        state->plugins[i]->win->has_input = true;
                        state->plugins[i]->win->input_title = input_titles[k];
                        state->plugins[i]->win->input_prompt = input_prompts[k];
                        break;
                    } else {
                        state->plugins[i]->win->has_input = false;
                    }
                }

                // file/buffer data
                for (int l = 0; l < num_win_file_plugins; l++) {

                    // file
                    if (i == win_file_data[l]) {
                        state->plugins[i]->win->has_data_buff = false;
                        state->plugins[i]->win->file_data = (data_file_t*) malloc (sizeof (data_file_t));
                        state->plugins[i]->win->buff_data = NULL;
                    } 

                    // buffer
                    else {
                        state->plugins[i]->win->has_data_buff = true;
                        state->plugins[i]->win->buff_data = (data_buff_t*) malloc (sizeof (data_buff_t));
                        state->plugins[i]->win->file_data = NULL;
                    }
                }
            } 

            // not window
            else {
                state->plugins[i]->has_window = false;
            }
        }
    }

    state->plugins[Src]->win->has_data_buff = true;
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



