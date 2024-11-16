#include <stdarg.h>
#include <pthread.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../display_lines.h"
#include "../utilities.h"

#include "get_assembly_data.h"
#include "get_breakpoint_data.h"
#include "get_debugger_output.h"
#include "get_local_vars.h"
#include "get_program_output.h"
#include "get_register_data.h"
#include "get_source_path_line_func.h"
#include "get_stack_data.h"
#include "get_watchpoint_data.h"
#include "../persist_data.h"

#define MAX_QUEUED_UPDATES  48

typedef struct {
    int updates [MAX_QUEUED_UPDATES];
    int front;
    int rear;
    int count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} update_queue_t;

update_queue_t update_queue;

static void  init_update_queue (void);
static void  enqueue_update    (update_queue_t *q, int plugin_index);



int
update_window (int plugin_index)
{
    enqueue_update (&update_queue, plugin_index);

    return A_OK;
}



int
update_windows (int num_updates, ...)
{
    int      i,
             plugin_index,
             ret;
    va_list  plugins;

    va_start (plugins, num_updates);
    for (i = 0; i < num_updates; i++) {
        plugin_index = va_arg (plugins, int);
        ret = update_window (plugin_index);
        if (ret == FAIL) {
            pfemr ("Update window loop failed");
        }
    }
    va_end (plugins);

    return A_OK;
}



static void
init_update_queue (void)
{
    update_queue_t q = update_queue;
    q.front = 0;
    q.rear = 0;
    q.count = 0;
    pthread_mutex_init (&q.lock, NULL);
    pthread_cond_init  (&q.not_empty, NULL);
    pthread_cond_init  (&q.not_full, NULL);
}



static void
enqueue_update (update_queue_t *q,
                int plugin_index)
{
    pthread_mutex_lock (&q->lock);

    while (q->count == MAX_QUEUED_UPDATES) {
        pthread_cond_wait (&q->not_full, &q->lock);
    }

    q->updates [q->rear] = plugin_index;
    q->rear = (q->rear + 1) % MAX_QUEUED_UPDATES;
    ++q->count;

    pthread_cond_signal(&q->not_empty);
    pthread_mutex_unlock (&q->lock);
}



static int
dequeue_update (update_queue_t *q)
{
    int update_index;

    pthread_mutex_lock (&q->lock);

    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->lock);
    }

    update_index = q->updates [q->front];
    q->front = (q->front + 1) % MAX_QUEUED_UPDATES;
    --q->count;

    pthread_cond_signal (&q->not_full);
    pthread_mutex_unlock (&q->lock);

    return update_index;
}



void*
update_window_thread (void *statev)
{
    int      plugin_index,
             ret;
    state_t *state;

    init_update_queue ();

    while (true) {

        plugin_index = dequeue_update (&update_queue);
        state = (state_t*) statev;
        state->debugger->curr_plugin_index = plugin_index;

        // get data
        switch (plugin_index) {
            case Asm:
                ret = get_assembly_data (state);
                if (ret == FAIL) {
                    pfem ("Failed to get assembly data");
                    goto upd_win_err;
                }
                break;
            case Brk: 
                ret = get_breakpoint_data (state);
                if (ret == FAIL) {
                    pfem ("Failed to get breakpoint data");
                    goto upd_win_err;
                }
                break;
            case Dbg:
                ret = get_debugger_output (state);
                if (ret == FAIL) {
                    pfem ("Failed to get debugger output");
                    goto upd_win_err;
                }
                break;
            case LcV:
                ret = get_local_vars (state);
                if (ret == FAIL) {
                    pfem ("Failed to get local variables");
                    goto upd_win_err;
                }
                break;
            case Prg:
                ret = get_program_output (state);
                if (ret == FAIL) {
                    pfem ("Failed to get program output");
                    goto upd_win_err;
                }
                break;
            case Reg:
                ret = get_register_data (state);
                if (ret == FAIL) {
                    pfem ("Failed to get register data");
                    goto upd_win_err;
                }
                break;
            case Src:
                ret = get_source_path_line_func (state);
                if (ret == FAIL) {
                    pfem ("Failed to get source, line, memory data");
                    goto upd_win_err;
                }
                break;
            case Stk:
                ret = get_stack_data (state);
                if (ret == FAIL) {
                    pfem ("Failed to get stack data");
                    goto upd_win_err;
                }
                break;
            case Wat:
                ret = get_watchpoint_data (state);
                if (ret == FAIL) {
                    pfem ("Failed to get watchpoint data");
                    goto upd_win_err;
                }
                break;
            default:
                pfem ("Unrecognized plugin index");
                goto upd_win_err;
        }

        // write to ncurses window
        ret = display_lines (state->plugins[plugin_index]->data_pos,
                             plugin_index,
                             state);
        if (ret == FAIL) {
            pfem (ERR_DISP_LINES);
            goto upd_win_err;
        }

        // persist data
        switch (plugin_index) {
            case Brk:
            case Wat:
                ret = persist_data (state);
                if (ret == FAIL) {
                    pfeme ("Failed to persist data");
                }
        }
    }

    return NULL;

upd_win_err:

    peme ("Failed to update window (index: %d, code: \"%s\", debugger: \"%s\")",
            plugin_index, get_plugin_code (plugin_index), state->debugger->title);
}

