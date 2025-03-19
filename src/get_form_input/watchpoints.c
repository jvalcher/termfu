#include <string.h>
#include <stdlib.h>

#include "watchpoints.h"
#include "_get_form_input.h"
#include "../data.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"
#include "../error.h"



int
insert_watchpoint (state_t *state)
{
    int index;
    watchpoint_t *watch = NULL;

    if (get_form_input ("Create watchpoint (variable): ", state->input_buffer) == FAIL)
        pfemr (ERR_POPUP_IN);

    if (strlen (state->input_buffer) > 0) {

        // first watchpoint
        if (state->watchpoints == NULL) {
            if ((state->watchpoints = (watchpoint_t*) malloc (sizeof (watchpoint_t))) == NULL) {
                pfemr_errno ("Failed to allocate watchpoint_t (var: \"%s\")", state->input_buffer);
            }
            watch = state->watchpoints;
            watch->index = 1;
            watch->var[0] = '\0';
            memcpy (watch->var, state->input_buffer, WATCH_LEN - 1);
            watch->var [WATCH_LEN - 1] = '\0';
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
            if ((watch->next = (watchpoint_t*) malloc (sizeof (watchpoint_t))) == NULL) {
                pfemr_errno ("Failed to allocate watchpoint_t (var: \"%s\")", state->input_buffer);
            }
            watch = watch->next;
            watch->index = index + 1;
            watch->value[0] = '\0';
            memcpy (watch->var, state->input_buffer, WATCH_LEN - 1);
            watch->var [WATCH_LEN - 1] = '\0';
            watch->next = NULL;
        }

        if (update_window (Wat) == FAIL)
            pfemr ("Failed to update watchpoint window");
    }

    return A_OK;
}



int
delete_watchpoint (state_t *state)
{
    watchpoint_t *prev_watch,
                 *watch;

    watch = state->watchpoints;
    prev_watch = watch;

    if (get_form_input ("Delete watchpoint (index): ", state->input_buffer) == FAIL)
        pfemr (ERR_POPUP_IN);

    while (watch != NULL) {

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

    if (update_window (Wat) == FAIL)
        pfemr (ERR_UPDATE_WIN);

    return A_OK;
}



int
clear_all_watchpoints (state_t *state)
{
    watchpoint_t *tmp_watch,
                 *watch;

    watch = state->watchpoints;
    while (watch != NULL) {
        tmp_watch = watch->next;
        free (watch);
        watch = tmp_watch; 
    }
    state->watchpoints = NULL;

    if (update_window (Wat) == FAIL)
        pfemr (ERR_UPDATE_WIN);

    return A_OK;
}

