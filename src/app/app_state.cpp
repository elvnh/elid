#include "app_state.h"

#define APP_PERMANENT_ARENA_SIZE     MB(32)
#define APP_PROCESS_LIST_ARENA_SIZE  MB(4)
#define APP_SCAN_RESULT_ARENA_SIZE   sizeof(scan_Match) * THOUSAND(100)
#define APP_PER_THREAD_SCAN_RESULT_ARENA_SIZE sizeof(scan_Match) * THOUSAND(10)
#define APP_WATCH_POOL_SIZE          4

b32 app_init(app_State *app)
{
    b32 result = false;

    app->permanent_arena = arena_create(APP_PERMANENT_ARENA_SIZE);
    app->process_list_arena = arena_create(APP_PROCESS_LIST_ARENA_SIZE);
    app->watch_pool = pool_create(app->permanent_arena, app_Watch, APP_WATCH_POOL_SIZE);
    app->error_list = app_error_list_create();

    // TODO: check number of cores on system
    // TODO: come up with a good distribution of number of threads between the different work queues
    usize thread_count = 16;
    app->per_frame_work_queue = wq_create(app->permanent_arena, 2);
    app->multi_frame_work_queue = wq_create(app->permanent_arena, (s32)thread_count);

    app->current_per_thread_scan_result_arenas  = arena_alloc_array(app->permanent_arena, Arena *, thread_count);
    app->previous_per_thread_scan_result_arenas = arena_alloc_array(app->permanent_arena, Arena *, thread_count);

    for (usize i = 0; i < wq_get_thread_count(app->multi_frame_work_queue); ++i) {
        app->current_per_thread_scan_result_arenas[i]  = arena_create(APP_PER_THREAD_SCAN_RESULT_ARENA_SIZE);
        app->previous_per_thread_scan_result_arenas[i] = arena_create(APP_PER_THREAD_SCAN_RESULT_ARENA_SIZE);
    }

    result = (app->permanent_arena != 0)
        && (app->process_list_arena != 0)
        && (app->per_frame_work_queue != 0)
        && (app->multi_frame_work_queue != 0)
        && (app->current_per_thread_scan_result_arenas != 0)
        && (app->previous_per_thread_scan_result_arenas != 0);

    if (result) {
        result = app_addr_table_init(app->permanent_arena, &app->address_table);
    }

    ASSERT(result);

    return result;
}

void app_destroy(app_State *app)
{
    // TODO: Destroy watches and clear breakpoints
    if (app_has_opened_process(app)) {
        memmi_close_process(app->open_process_handle, memmi_default_allocator());
    }

    app_error_list_destroy(&app->error_list);

    if (app->watch) {
        TODO("app_watch_destroy()");
        // app_watch_destroy(app->watch);
    }

    usize thread_count = wq_get_thread_count(app->multi_frame_work_queue);
    wq_destroy(app->per_frame_work_queue);
    wq_destroy(app->multi_frame_work_queue);

    for (usize i = 0; i < thread_count; ++i) {
        arena_destroy(app->current_per_thread_scan_result_arenas[i]);
        arena_destroy(app->previous_per_thread_scan_result_arenas[i]);
    }

    arena_destroy(app->permanent_arena);
    arena_destroy(app->process_list_arena);
}

void app_push_error(app_State *app, app_Error error)
{
    ScopedLock lock = scoped_lock(app->error_list.mutex);

    if (NEVER(app->error_list.count >= array_capacity(app->error_list.data))) {
        ASSERT(app->error_list.count == array_capacity(app->error_list.data));

        app->error_list.count = array_capacity(app->error_list.data) - 1;
    }

    array_push(&app->error_list, error);
}

b32 app_has_opened_process(app_State *app)
{
    b32 result = !memmi_process_is_null(app->open_process_handle);

    return result;
}

b32 app_result_table_is_stale(app_State *app)
{
    b32 result =
        (atomic_load_s32(&app->scanner_state) == APP_SCANNER_STATE_SWITCHED_PROCESS_AFTER_FIRST_SCAN)
        || (!atomic_load_s32(&app->scan_in_progress) && (app->open_process_handle.pid != app->result_list.scanned_process.pid));

    return result;
}
