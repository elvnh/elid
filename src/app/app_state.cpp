#include "app_state.h"

#define APP_PERMANENT_ARENA_SIZE     MB(32)
#define APP_PROCESS_LIST_ARENA_SIZE  MB(4)
#define APP_SCAN_RESULT_ARENA_SIZE   sizeof(scan_Match) * THOUSAND(100)
#define APP_WATCH_POOL_SIZE          4

b32 app_init(app_State *app)
{
    b32 result = false;

    app->permanent_arena = arena_create(APP_PERMANENT_ARENA_SIZE);
    app->process_list_arena = arena_create(APP_PROCESS_LIST_ARENA_SIZE);
    app->scan_result_arena = arena_create(APP_SCAN_RESULT_ARENA_SIZE);
    app->previous_scan_result_arena = arena_create(APP_SCAN_RESULT_ARENA_SIZE);
    app->watch_pool = pool_create(app->permanent_arena, app_Watch, APP_WATCH_POOL_SIZE);

    // TODO: check number of cores on system
    // TODO: come up with a good distribution of number of threads between the different work queues
    s32 thread_count = 16;
    app->per_frame_work_queue = app_wq_create(app->permanent_arena, thread_count);
    app->multi_frame_work_queue = app_wq_create(app->permanent_arena, thread_count);

    result = (app->permanent_arena != 0)
        && (app->process_list_arena != 0)
        && (app->scan_result_arena != 0)
        && (app->previous_scan_result_arena != 0)
        && (app->per_frame_work_queue != 0)
        && (app->multi_frame_work_queue != 0);

    if (result) {
        result = app_addr_table_init(app->permanent_arena, &app->address_table);
    }

    ASSERT(result);

    return result;
}

void app_destroy(app_State *app)
{
    (void)app;
    // app_wq_destroy(app->per_frame_work_queue);
    // app_wq_destroy(app->multi_frame_work_queue);
}

void app_push_error(app_State *app, app_Error error)
{
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
