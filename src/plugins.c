/*
    See README.md for more information
*/

#include <string.h>
#include <errno.h>
#include <time.h>

#include "plugins.h"
#include "data.h"
#include "utilities.h"


/*
    Plugin codes
    -------
    Must be in alphabetical order [A-Z,a-z]
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
    "Nxt",
    "Prg",
    "Prm",
    "Qut",
    "Reg",
    "Run",
    "Src",
    "Stk",
    "Stp",
    "Trg",
    "Unt",
    "Wat"
};



int win_plugins[]      = { Asm, Brk, Dbg, LcV, Prg, Reg, Src, Stk, Wat };
int win_file_plugins[] = { Src };

int win_buff_plugins[] = {
    Asm,
    Brk,
    Dbg,
    LcV,
    Prg,
    Reg,
    Stk,
    Wat
};

int win_topbar_plugins[] = {
    Brk,
    Src,
    Wat
};
char *win_topbar_titles[] = {
    "(c)reate  (d)elete  clear (a)ll",
    "",                                     // set dynamically
    "(c)reate  (d)elete  clear (a)ll"
};



int
allocate_plugin_windows (state_t *state)
{
    int i, j,
        num_win_plugins       = sizeof (win_plugins) / sizeof (win_plugins[0]),
        num_win_topbar_plugins = sizeof (win_topbar_plugins) / sizeof (win_topbar_plugins[0]),
        num_win_file_plugins  = sizeof (win_file_plugins) / sizeof (win_file_plugins[0]),
        num_win_buff_plugins  = sizeof (win_buff_plugins) / sizeof (win_buff_plugins[0]);

    window_t *win;
    plugin_t *plugin;

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
        win->has_topbar = false;
        win->key = j;
        memcpy (win->code, plugin_codes[j], CODE_LEN + 1);
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

    // src_file_data_t
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

        if ((win->src_file_data = (src_file_data_t*) malloc (sizeof (src_file_data_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("file_data_t allocation error (code: %s)", win->code);
        }

        win->src_file_data->path_changed = true;
        win->src_file_data->path[0]  = '\0';
        win->src_file_data->path_len = FILE_PATH_LEN;
        win->src_file_data->path_pos = 0;
        win->src_file_data->func[0]  = '\0';
        win->src_file_data->func_len = FUNC_LEN;
        win->src_file_data->func_pos = 0;
        win->src_file_data->addr[0]  = '\0';
        win->src_file_data->addr_len = ADDRESS_LEN;
        win->src_file_data->addr_pos = 0;
        win->buff_data = NULL;
    }

    // buff_data_t
    for (i = 0; i < num_win_buff_plugins; i++) {
        
        j = win_buff_plugins[i];
        win = state->plugins[j]->win;
        state->plugins[j]->win_type = BUFF_TYPE;

        // set display position
        switch (j) {
            case Asm:
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
        }

        if ((win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t))) == NULL) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("buff_data_t allocation error (code: %s)", win->code);
        }
        if ((win->buff_data->buff = (char*) malloc (sizeof(char) * ORIG_BUF_LEN)) == NULL ) {
            pfem ("malloc error: %s", strerror (errno));
            pemr ("buff_data_t->buff (code: %s) allocation error", win->code);
        }

        memcpy (win->buff_data->code, plugin_codes[j], CODE_LEN + 1);
        win->buff_data->buff_pos = 0;
        win->buff_data->buff_len = ORIG_BUF_LEN;
        win->buff_data->times_doubled = 0;
        win->buff_data->scroll_col = 1;
        win->buff_data->scroll_row = 1;
        win->buff_data->changed = true;
        win->src_file_data = NULL;
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
