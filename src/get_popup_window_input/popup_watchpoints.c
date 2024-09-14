#include <string.h>

#include "popup_watchpoints.h"
#include "_get_popup_window_input.h"
#include "../data.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"



void
insert_watchpoint (state_t *state)
{
    int index;
    watchpoint_t *watch = NULL;

    get_popup_window_input  ("Create watchpoint (variable): ", state->input_buffer);

    if (strlen (state->input_buffer) > 0) {

        // first watchpoint
        if (state->watchpoints == NULL) {
            state->watchpoints = (watchpoint_t*) malloc (sizeof (watchpoint_t));
            watch = state->watchpoints;
            watch->index = 1;
            watch->var[0] = '\0';
            strncpy (watch->var, state->input_buffer, WATCH_LEN - 1);
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
            strncpy (watch->var, state->input_buffer, WATCH_LEN - 1);
            watch->next = NULL;
        }
    }

    update_window (Wat, state);
}



void
delete_watchpoint (state_t *state)
{
    watchpoint_t *prev_watch,
                 *watch;

    watch = state->watchpoints;
    prev_watch = watch;

    get_popup_window_input  ("Delete watchpoint (index): ", state->input_buffer);

    while (watch != NULL) {

        // delete watchpoint
        if (watch->index == atoi (state->input_buffer)) {
            if (watch == state->watchpoints) {
                state->watchpoints = watch->next;
            } else {
                prev_watch->next = watch->next;
            }
            free (watch);
            break;
        }

        prev_watch = watch;
        watch = watch->next; 
    }

    update_window (Wat, state);
}

