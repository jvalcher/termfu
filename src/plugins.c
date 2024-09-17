/*

Plugins
------
- Each case-sensitive, three-letter code is associated with a plugin
- Each plugin is assigned a key shortcut [A-Z,a-z] in the configuration file

- Codes:

    - Window

        Asm     Assembly code
        Brk     Breakpoints
        Dbg     Debugger output
        LcV     Local variables
        Prg     Program output
        Reg     Registers
        Src     Source file
        Wat     Watchpoints

    - Pop-up window selection

        Lay     Layouts
        Prm     Execute custom debugger command
        Unt     Execute until

    - Non-window

        EMP     empty index
        Qut     Quit
        Con     Continue
        Fin     Finish
        Kil     Kill
        Nxt     Next
        Run     (Re)run program
        Stp     Step
*/

#include <string.h>
#include <time.h>

#include "plugins.h"
#include "data.h"
#include "utilities.h"
#include "display_lines.h"
#include "update_window_data/_update_window_data.h"



/*
    Plugin codes
*/
char *plugin_codes [] = {
    
    "EMP",
    "Asm",
    "Brk",
    "Con",
    "Dbg",
    "Fin",
    "Kil",
    "Lay",
    "LcV",
    "Nxt",
    "Prg",
    "Prm",
    "Qut",
    "Reg",
    "Run",
    "Src",
    "Stp",
    "Unt",
    "Wat"
};



int win_plugins[]      = { Asm, Brk, Dbg, LcV, Prg, Reg, Src, Wat };
int win_file_plugins[] = { Src };

int win_buff_plugins[] = {
    Asm,
    Brk,
    Dbg,
    LcV,
    Prg,
    Reg,
    Wat
};
int win_buff_len[] = {
    Asm_BUF_LEN,
    Brk_BUF_LEN,
    Dbg_BUF_LEN,
    LcV_BUF_LEN,
    Prg_BUF_LEN,
    Reg_BUF_LEN,
    Wat_BUF_LEN
};

int win_input_plugins[] = {
    Brk,
    Src,
    Wat
};
char *win_input_titles[] = {
    "(c)reate  (d)elete  clear (a)ll",
    "",                                     // set dynamically
    "(c)reate  (d)elete  clear (a)ll"
};



void
allocate_plugin_windows (state_t *state)
{
    int i, j,
        num_win_plugins       = sizeof (win_plugins) / sizeof (win_plugins[0]),
        num_win_input_plugins = sizeof (win_input_plugins) / sizeof (win_input_plugins[0]),
        num_win_file_plugins  = sizeof (win_file_plugins) / sizeof (win_file_plugins[0]),
        num_win_buff_plugins  = sizeof (win_buff_plugins) / sizeof (win_buff_plugins[0]);

    window_t *win;
    plugin_t *plugin;

    // window_t structs
    for (i = 0; i < num_win_plugins; i++) {

        j = win_plugins[i];
        plugin = state->plugins[j];
        plugin->win = (window_t*) malloc (sizeof (window_t));
        win = plugin->win;
        if (win == NULL) {
            pfeme ("Window malloc failed for plugin %s (code: %s, index: %d)\n",
                    plugin->title, plugin->code, i);
        }
        win->has_topbar = false;
        win->key = j;
        strcpy (win->code, plugin_codes[j]);
    }

    // topbar subwindow data
    for (i = 0; i < num_win_input_plugins; i++) {
        j = win_input_plugins[i];
        win = state->plugins[j]->win;
            //
        win->has_topbar = true;

        win->topbar_title = (char*) malloc (strlen (win_input_titles[i]) + 1);
        if (win->topbar_title == NULL) {
            pfeme ("Unable to allocate window input title (\"%s\")\n", win_input_titles[i]);
        }
        strcpy (win->topbar_title, win_input_titles[i]);
    }

    // file_data_t
    for (i = 0; i < num_win_file_plugins; i++) {
        j = win_file_plugins[i];

        // set initial display position
        switch (j) {
            case Src:
                state->plugins[j]->data_pos = LINE_DATA;
                break;
            default:
                state->plugins[j]->data_pos = LINE_DATA;
        }
        win = state->plugins[j]->win;

        state->plugins[j]->win_type = FILE_TYPE;
        win->has_data_buff = false;

        win->file_data = (file_data_t*) malloc (sizeof (file_data_t));
        if (win->file_data == NULL) {
            pfeme ("Unable to allocate file_data_t (%s)\n", win->code);
        }

        win->file_data->path_changed = true;
        win->file_data->path[0]  = '\0';
        win->file_data->path_len = FILE_PATH_LEN;
        win->file_data->path_pos = 0;
        win->file_data->func[0]  = '\0';
        win->file_data->func_len = FUNC_LEN;
        win->file_data->func_pos = 0;
        win->file_data->addr[0]  = '\0';
        win->file_data->addr_len = ADDRESS_LEN;
        win->file_data->addr_pos = 0;
        win->buff_data = NULL;
    }

    // buff_data_t
    for (i = 0; i < num_win_buff_plugins; i++) {
        
        j = win_buff_plugins[i];
        win = state->plugins[j]->win;
        state->plugins[j]->win_type = BUFF_TYPE;

        // set initial display position
        switch (j) {
            case Asm:
                state->plugins[j]->data_pos = ROW_DATA;
                break;
            case Brk: 
            case LcV:
            case Reg:
            case Wat:
                state->plugins[j]->data_pos = BEG_DATA;
                break;
            case Dbg:
            case Prg:
                state->plugins[j]->data_pos = END_DATA;
                break;
        }

        win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
        if (win->buff_data == NULL) {
            pfeme ("Unable to allocate buff_data_t (%s)\n", win->code);
        }
        win->buff_data->buff = (char*) malloc (win_buff_len[i]);
        if (win->buff_data == NULL) {
            pfeme ("Unable to allocate buff_data_t->buff (%s)\n", win->code);
        }

        win->has_data_buff = true;
        win->buff_data->buff_pos = 0;
        win->buff_data->buff_len = win_buff_len[i];
        win->buff_data->scroll_col = 1;
        win->buff_data->scroll_row = 1;
        win->buff_data->changed = true;
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



