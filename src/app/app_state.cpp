#include "app_state.h"

#define APP_PERMANENT_ARENA_SIZE     MB(32)
#define APP_PROCESS_LIST_ARENA_SIZE  MB(4)
#define APP_SCAN_RESULT_ARENA_SIZE   sizeof(scan_Match) * THOUSAND(100)

b32 app_init(app_State *app)
{
    b32 result = false;

    app->permanent_arena = arena_create(APP_PERMANENT_ARENA_SIZE);
    app->process_list_arena = arena_create(APP_PROCESS_LIST_ARENA_SIZE);
    app->scan_result_arena = arena_create(APP_SCAN_RESULT_ARENA_SIZE);
    app->previous_scan_result_arena = arena_create(APP_SCAN_RESULT_ARENA_SIZE);

    // TODO: check number of cores on system
    s32 thread_count = 16;
    app->work_queue = app_wq_create(app->permanent_arena, thread_count);

    // Initialize thread local contexts.
    // TODO: when we multithread, ensure that all threads do this
    ThreadContext *thread_context = thread_ctx_init();
    ASSERT(thread_context);

    result = (app->permanent_arena != 0)
        && (app->process_list_arena != 0)
        && (app->scan_result_arena != 0)
        && (app->previous_scan_result_arena != 0)
        && (app->work_queue != 0)
        && (thread_context != 0);

    if (result) {
        result = app_addr_table_init(app->permanent_arena, &app->address_table);
    }

    ASSERT(result);

    return result;
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
