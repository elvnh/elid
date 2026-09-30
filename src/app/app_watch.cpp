#include "app_watch.h"

#define APP_WATCH_TIMEOUT_MS       10
#define APP_WATCH_EVENT_ARENA_SIZE MB(1)

void app_watch__loop(app_TaskID, void *user_data)
{
    app_Watch *watch = (app_Watch *)user_data;

    while (!atomic_load_s32(&watch->should_terminate)) {
        memmi_Process proc = watch->app->open_process_handle;
        memmi_DebugEvent event = memmi_wait_for_debug_event(proc, MEMMI_CONTINUE_UNHANDLED, APP_WATCH_TIMEOUT_MS);

        if (event.status != MEMMI_OK) {
            ASSERT(0);
        } else {
            if (event.kind == MEMMI_DEBUG_EVENT_BREAKPOINT) {
                b32 breakpoint_is_ours = event.as.breakpoint.breakpoint_index == watch->breakpoint_index;

                if (breakpoint_is_ours) {
                    printf("breakpoint %zu\n", event.as.breakpoint.ip_register);
                }
            }
        }
    }
}

app_Watch *app_watch_begin(app_State *app, uintptr_t address, app_WatchCondition cond,
    u32 size, u32 index)
{
    // NOTE: must be attached when attempting this
    app_Watch *result = 0;

    memmi_BreakpointCondition memmi_cond = {};

    if (cond == APP_WATCH_COND_WRITE) {
        memmi_cond = MEMMI_BREAKPOINT_WRITE;
    } else {
        ASSERT(cond == APP_WATCH_COND_READ_WRITE);
        memmi_cond = MEMMI_BREAKPOINT_READ_WRITE;
    }

    memmi_Status breakpoint_result = memmi_set_hardware_breakpoint(app->open_process_handle,
        address, memmi_cond, index, (memmi_BreakpointLength)size);

    if (breakpoint_result == MEMMI_OK) {
        result = (app_Watch *)calloc(1, sizeof(app_Watch));
        result->list_mutex = mutex_create();
        result->event_arena = arena_create(APP_WATCH_EVENT_ARENA_SIZE);
        result->app = app;
        result->task = app_wq_push_task(app->work_queue, app_watch__loop, result);
    }

    return result;
}

void app_watch_destroy(app_Watch *watch)
{
    atomic_store_s32(&watch->should_terminate, true);

    app_wq_wait_for_task(watch->task);

    arena_destroy(watch->event_arena);

    mutex_destroy(watch->list_mutex);
    free(watch);
}

app_WatchEvent *app_pop_event(app_Watch *watch)
{
    // TODO: use a lockless queue instead
    ScopedLock lock = scoped_lock(watch->list_mutex);

    app_WatchEvent *result = watch->event_list.first;
    queue_pop(&watch->event_list);

    return result;
}
