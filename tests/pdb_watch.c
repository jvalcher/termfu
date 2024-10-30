#include "../src/data.h"
#include "../src/start_debugger.h"
#include "../src/insert_output_marker.h"
#include "../src/utilities.h"
#include "../src/parse_debugger_output.h"
#include "../src/update_window_data/get_watchpoint_data.h"
#include "../src/plugins.h"

static void insert_watchpoint_pdb (state_t *state, char *var);



int
main (void)
{
    //////////// allocate structs
    //////////// set plugin_index variables

    int plugin_index = Wat;

    state_t *state = (state_t*) malloc (sizeof (state_t));
    set_state_ptr (state);
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    set_num_plugins (state);
    allocate_plugins (state);
    allocate_plugin_windows (state);

    debugger_t *debugger   = state->debugger;
    plugin_t *plugin       = state->plugins[plugin_index];
    window_t *win          = plugin->win;
    //buff_data_t *buff_data = win->buff_data;

    ////////////

    watchpoint_t *watch;

    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;
    debugger->index = DEBUGGER_PDB;

    start_debugger (state);

    // no watchpoints
    get_watchpoint_data (state);
    putchar ('\n');
    printf ("No watchpoints: \n\"\n%s\n\"\n\n", win->buff_data->buff);

    // create watchpoints
    insert_watchpoint_pdb (state, "num1");
    insert_watchpoint_pdb (state, "narr");
    insert_watchpoint_pdb (state, "str1");

        // print
    watch = state->watchpoints;
    printf ("watchpoint_t: \n");
    do {
        printf ("(%d) %s\n", watch->index, watch->var);
        watch = watch->next;
    } while (watch != NULL);
    putchar ('\n');

    // watchpoints, not running
    get_watchpoint_data (state);
    printf ("Watchpoints, not running: \n\"\n%s\n\"\n\n", win->buff_data->buff);

    // watchpoints, running
    send_command_mp (state, "break 10\n");
    send_command_mp (state, "continue\n");
    get_watchpoint_data (state);
    printf ("Watchpoints, running: \n\"\n%s\n\"\n\n", win->buff_data->buff);

    return 0;
}



static void
insert_watchpoint_pdb (state_t *state, char *var)
{
    int index;
    watchpoint_t *watch = NULL;

    // first watchpoint
    if (state->watchpoints == NULL) {
        state->watchpoints = (watchpoint_t*) malloc (sizeof (watchpoint_t));
        watch = state->watchpoints;
        watch->index = 1;
        watch->var[0] = '\0';
        strncpy (watch->var, var, WATCH_LEN - 1);
        watch->next = NULL;
    } 

    // new watchpoint
    else {
        watch = state->watchpoints;
        index = watch->index;
        while (watch->next != NULL) {
            watch = watch->next; 
            index = watch->index;
        }
        watch->next = (watchpoint_t*) malloc (sizeof (watchpoint_t));
        watch = watch->next;
        watch->index = index + 1;
        watch->value[0] = '\0';
        strncpy (watch->var, var, WATCH_LEN - 1);
        watch->next = NULL;
    }
}

