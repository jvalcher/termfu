#include <stdarg.h>
#include <pthread.h>

#include "_update_window_data.h"
#include "../data.h"
#include "../plugins.h"
#include "../display_lines.h"
#include "../error.h"

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
#define UPD_WIN_STATE  " (index: %d, code: \"%s\", debugger: \"%s\")", \
                        plugin_index, get_plugin_code (plugin_index), state->debugger->title

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
update_windows_impl (int num_updates, ...)
{
    int      plugin_index;
    va_list  plugins;

    va_start (plugins, num_updates);
    for (plugin_index = va_arg (plugins, int);
         plugin_index != NULL_PLUGIN;
         plugin_index = va_arg (plugins, int))
    {
        if (update_window (plugin_index) == FAIL)
            pfemr ("Update window loop failed");
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
    int      plugin_index;
    state_t *state;

    init_update_queue ();

    while (true) {

        plugin_index = dequeue_update (&update_queue);
        state = (state_t*) statev;
        state->debugger->curr_plugin_index = plugin_index;

        // get data
        switch (plugin_index) {
            case Asm:
                if (get_assembly_data (state) == FAIL)
                    pfemn ("Failed to get assembly data" UPD_WIN_STATE);
                break;
            case Brk: 
                if (get_breakpoint_data (state) == FAIL)
                    pfemn ("Failed to get breakpoint data" UPD_WIN_STATE);
                break;
            case Dbg:
                if (get_debugger_output (state) == FAIL)
                    pfemn ("Failed to get debugger output" UPD_WIN_STATE);
                break;
            case LcV:
                if (get_local_vars (state) == FAIL)
                    pfemn ("Failed to get local variables" UPD_WIN_STATE);
                break;
            case Prg:
                if (get_program_output (state) == FAIL)
                    pfemn ("Failed to get program output" UPD_WIN_STATE);
                break;
            case Reg:
                if (get_register_data (state) == FAIL)
                    pfemn ("Failed to get register data" UPD_WIN_STATE);
                break;
            case Src:
                if (get_source_path_line_func (state) == FAIL)
                    pfemn ("Failed to get source, line, memory data" UPD_WIN_STATE);
                break;
            case Stk:
                if (get_stack_data (state) == FAIL)
                    pfemn ("Failed to get stack data" UPD_WIN_STATE);
                break;
            case Wat:
                if (get_watchpoint_data (state) == FAIL)
                    pfemn ("Failed to get watchpoint data" UPD_WIN_STATE);
                break;
            default:
                pfemn ("Unrecognized plugin index" UPD_WIN_STATE);
        }

        // write to ncurses window
        if (display_lines (state->plugins[plugin_index]->data_pos,
                           plugin_index,
                           state) == FAIL)
            pfemn (ERR_DISP_LINES UPD_WIN_STATE);

        // persist data
        switch (plugin_index) {
            case Brk:
            case Wat:
                if (persist_data (state) == FAIL)
                    pfemn ("Failed to persist data" UPD_WIN_STATE);
        }
    }

    return NULL;
}

