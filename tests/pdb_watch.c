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
    state_t *state = (state_t*) malloc (sizeof (state_t));
    state->debugger = (debugger_t*) malloc (sizeof (debugger_t));
    state->plugins = (plugin_t**) malloc (sizeof (plugin_t*));
    state->plugins[Wat] = (plugin_t*) malloc (sizeof (plugin_t));
    state->plugins[Wat]->win = (window_t*) malloc (sizeof (window_t));
    window_t *win = state->plugins[Wat]->win;
    win->buff_data = (buff_data_t*) malloc (sizeof (buff_data_t));
    win->buff_data->buff = (char*) malloc (sizeof (char) * Wat_BUF_LEN);
    watchpoint_t *watch;

    win->buff_data->buff_len = Wat_BUF_LEN;
    state->debugger->index = DEBUGGER_PDB;
    char *cmd[] = {"python3", "-m", "pdb", "../misc/gcd.py", NULL };
    state->command = cmd;

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
    insert_output_start_marker (state);
    send_command (state, "break 10\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

    insert_output_start_marker (state);
    send_command (state, "continue\n");
    insert_output_end_marker (state);
    parse_debugger_output (state);

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

