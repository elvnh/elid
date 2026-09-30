#include "app_watch.h"

#define APP_WATCH_TIMEOUT_MS       10
#define APP_WATCH_ARENA_SIZE       MB(1)
#define APP_WATCH_EVENT_POOL_SIZE  128

void app_watch__loop(app_TaskID, void *user_data)
{
    app_WatchThreadContext *watch = (app_WatchThreadContext *)user_data;

    while (!atomic_load_s32(&watch->should_terminate)) {
        memmi_Process proc = watch->process;
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

app_Watch app_watch_begin(app_WorkQueue *wq, memmi_Process proc, uintptr_t address, app_WatchCondition cond, u32 size, u32 index)
{
    // NOTE: must be attached when attempting this
    app_Watch result = {};

    memmi_BreakpointCondition memmi_cond = {};

    if (cond == APP_WATCH_COND_WRITE) {
        memmi_cond = MEMMI_BREAKPOINT_WRITE;
    } else {
        ASSERT(cond == APP_WATCH_COND_READ_WRITE);
        memmi_cond = MEMMI_BREAKPOINT_READ_WRITE;
    }

    memmi_Status breakpoint_result = memmi_set_hardware_breakpoint(
        proc, address, memmi_cond, index, (memmi_BreakpointLength)size);

    if (breakpoint_result == MEMMI_OK) {
        result.ok = true;
        result.arena = arena_create(APP_WATCH_ARENA_SIZE);
        result.thread_context.process = proc;
        result.thread_context.event_queue.mutex = mutex_create();
        result.thread_context.event_queue.pool = pool_create(result.arena, app_WatchEvent, APP_WATCH_EVENT_POOL_SIZE);

        app_WatchThreadContext *watch_thread_arg = &result.thread_context;
        result.task = app_wq_push_task(wq, app_watch__loop, watch_thread_arg);
    }

    return result;
}

void app_watch_destroy(app_Watch *watch)
{
    atomic_store_s32(&watch->thread_context.should_terminate, true);

    app_wq_wait_for_task(watch->task);

    mutex_destroy(watch->thread_context.event_queue.mutex);

    arena_destroy(watch->arena);

    *watch = {};
}

void app_watch_poll_events(app_Watch *watch)
{
    ScopedLock lock = scoped_lock(watch->thread_context.event_queue.mutex);

    app_WatchEvent *head_event = watch->thread_context.event_queue.first;

    // TODO: list_clear macro
    watch->thread_context.event_queue.first = 0;
    watch->thread_context.event_queue.last = 0;
    watch->thread_context.event_queue.list_node_count = 0;

    for (app_WatchEvent *e = head_event; e; e = list_next(e)) {
        app_WatchInstruction *instr = arena_alloc_struct(watch->arena, app_WatchInstruction);
        // TODO: initialize
        // TODO: check if this instruction has been triggered already

        dll_push_back(&watch->triggered_instruction_list, instr);
    }

    pool_reset(watch->thread_context.event_queue.pool);
}
