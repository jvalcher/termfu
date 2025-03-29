/*
    See README.md for more information
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "plugins.h"
#include "data.h"
#include "utilities.h"


/*
    Plugin codes
    -------
    - Case sensitive
    - MUST be sorted alphabetically to match enums in plugins.h
*/
char *plugin_codes [] = {
    "Asm",
    "AtP",
    "Brk",
    "Con",
    "Dbg",
    "Fin",
    "Kil",
    "Lay",
    "LcV",
    "Nxi",
    "Nxt",
    "Prg",
    "Prm",
    "Qut",
    "Reg",
    "Run",
    "Src",
    "Sti",
    "Stk",
    "Stp",
    "Unt",
    "Wat"
};



int
allocate_plugins (state_t *state)
{
    // state->plugins
    if ((state->plugins = (plugin_t**) malloc (state->num_plugins * sizeof (plugin_t*))) == NULL) {
        pfem ("malloc error: %s", strerror (errno));
        pemr ("Failed to allocate plugin array for %d plugins", state->num_plugins);
    }

    // state->plugins[i]
    for (int i = 0; i < state->num_plugins; i++) {
        if ((state->plugins [i] = (plugin_t*) malloc (sizeof (plugin_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("plugin_t pointer allocation failed (index: %d, code: %s)", i, get_plugin_code (i));
        }
    }

    return A_OK;
}



// plugins with Ncurses WINDOWs
int win_plugins[]      = { Asm, Brk, Dbg, LcV, Prg, Reg, Src, Stk, Wat };

// window plugins with a topbar
int win_topbar_plugins[] = {
    Brk,
    Src,
    Wat
};
char *win_topbar_titles[] = {
    "(c)reate  (d)elete  clear (a)ll",
    "",                                     // current source file
    "(c)reate  (d)elete  clear (a)ll"
};



int
allocate_plugin_windows (state_t *state)
{
    window_t    *win;
    buff_data_t *buff_data;
    plugin_t    *plugin;

    int i, j,
        num_win_plugins       = sizeof (win_plugins) / sizeof (win_plugins[0]),
        num_win_topbar_plugins = sizeof (win_topbar_plugins) / sizeof (win_topbar_plugins[0]);

    // window_t structs
    for (i = 0; i < num_win_plugins; i++) {

        j = win_plugins[i];
        plugin = state->plugins[j];
        if ((plugin->win = (window_t*) malloc (sizeof (window_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("window_t allocation error (title: \"%s\", code: %s, index: %d)",
                    plugin->title, plugin->code, i);
        }
        win = plugin->win;
        memcpy (win->code, plugin_codes[j], CODE_LEN + 1);
        win->index = j;
        win->has_topbar = false;

        // data display position
        switch (j) {
            case Asm:
            case Src:
                state->plugins[j]->data_pos = ROW_DATA;
                break;
            case Brk: 
            case LcV:
            case Reg:
            case Stk:
            case Wat:
                state->plugins[j]->data_pos = BEG_DATA;
                break;
            case Dbg:
            case Prg:
                state->plugins[j]->data_pos = END_DATA;
                break;
            default:
                state->plugins[j]->data_pos = BEG_DATA;
        }

        // allocate buff_data_t, ->buff
        if ((win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t))) == NULL) {
            pfem ("malloc error: \"%s\"", strerror (errno));
            pemr ("Failed to allocate buff_data_t (code: \"%s\")", win->code);
        }
        if ((win->buff_data->buff = (char*) malloc (sizeof(char) * ORIG_BUF_LEN)) == NULL ) {
            pfem ("malloc error: \"%s\"", strerror (errno));
            pemr ("Failed to allocate buff_data->buff (code: \"%s\")", win->code);
        }
        buff_data = win->buff_data;

        // text wrapping
        switch (j) {
            case Asm:
            case Reg:
            case Src:
                buff_data->text_wrapped = false;
                break;
            default:
                buff_data->text_wrapped = true;
        }

        memcpy (buff_data->code, plugin_codes[j], CODE_LEN + 1);
        buff_data->buff_pos      = 0;
        buff_data->buff_len      = ORIG_BUF_LEN;
        buff_data->times_doubled = 0;
        buff_data->changed       = true;
    }

    // topbar subwindow data
    for (i = 0; i < num_win_topbar_plugins; i++) {
        j = win_topbar_plugins[i];
        win = state->plugins[j]->win;
            //
        win->has_topbar = true;

        if ((win->topbar_title = (char*) malloc (strlen (win_topbar_titles[i]) + 1)) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("win->topbar_title allocation error (code: %s, title: %s)",
                        win->code, win_topbar_titles[i]);
        }
        strcpy (win->topbar_title, win_topbar_titles[i]);
    }

    return A_OK;
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
    int si = 0,
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

    pfemr ("Failed to find index for plugin code \"%s\"", code);
}



char*
get_plugin_code (int plugin_index)
{
    int num_plugins = sizeof (plugin_codes) / sizeof (plugin_codes [0]);

    if (plugin_index < num_plugins && plugin_index >= 0) {
        return plugin_codes [plugin_index];
    }

    pfem ("Failed to get plugin code for index \"%d\"", plugin_index);
    return NULL;
}



void
print_plugin_indexes_codes (void)
{
    int num_plugins = sizeof (plugin_codes) / sizeof (plugin_codes[0]),
        spaces, n;

    for (int i = 0; i < num_plugins; i++) {

        n = i;
        spaces = 4;
        if (n == 0) {
            --spaces;
        } else {
            while (n != 0) {
                n /= 10;
                --spaces;
            }
        }

        printf ("\033[0;32m%d\033[0m%*c%s\n", i, spaces, ' ', plugin_codes[i]);
    }
}
