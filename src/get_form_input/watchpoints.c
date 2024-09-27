#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "watchpoints.h"
#include "_get_form_input.h"
#include "../data.h"
#include "../update_window_data/_update_window_data.h"
#include "../plugins.h"
#include "../utilities.h"



int
insert_watchpoint (state_t *state)
{
    int index,
        ret;
    watchpoint_t *watch = NULL;

    ret = get_form_input ("Create watchpoint (variable): ", state->input_buffer);
    if (ret == FAIL) {
        pfemr (ERR_POPUP_IN);
    }

    if (strlen (state->input_buffer) > 0) {

        // first watchpoint
        if (state->watchpoints == NULL) {
            if ((state->watchpoints = (watchpoint_t*) malloc (sizeof (watchpoint_t))) == NULL) {
                pfem ("malloc error: %s", strerror (errno));
                pemr ("Failed to allocate watchpoint_t (var: \"%s\")", state->input_buffer);
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
                pfem ("malloc error: %s", strerror (errno));
                pemr ("Failed to allocate watchpoint_t (var: \"%s\")", state->input_buffer);
            }
            watch = watch->next;
            watch->index = index + 1;
            watch->value[0] = '\0';
            memcpy (watch->var, state->input_buffer, WATCH_LEN - 1);
            watch->var [WATCH_LEN - 1] = '\0';
            watch->next = NULL;
        }
    }

    ret = update_window (Wat, state);
    if (ret == FAIL) {
        pfemr ("Failed to update watchpoint window");
    }

    return A_OK;
}



int
delete_watchpoint (state_t *state)
{
    int ret;
    watchpoint_t *prev_watch,
                 *watch;

    watch = state->watchpoints;
    prev_watch = watch;

    ret = get_form_input ("Delete watchpoint (index): ", state->input_buffer);
    if (ret == FAIL) {
        pfemr (ERR_POPUP_IN);
    }

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

    ret = update_window (Wat, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    return A_OK;
}



int
clear_all_watchpoints (state_t *state)
{
    int ret;
    watchpoint_t *tmp_watch,
                 *watch;

    watch = state->watchpoints;
    while (watch != NULL) {
        tmp_watch = watch->next;
        free (watch);
        watch = tmp_watch; 
    }
    state->watchpoints = NULL;

    ret = update_window (Wat, state);
    if (ret == FAIL) {
        pfemr (ERR_UPDATE_WIN);
    }

    return A_OK;
}

