#include "app_state.h"

#define APP_PERMANENT_ARENA_SIZE MB(32)
#define FRAME_ARENA_SIZE         MB(8)
#define PROCESS_LIST_ARENA_SIZE  MB(4)
#define SCAN_RESULT_ARENA_SIZE   sizeof(scan_Match) * THOUSAND(100)

b32 app_init(app_State *app)
{
    b32 result = false;

    app->permanent_arena = arena_create(APP_PERMANENT_ARENA_SIZE);
    app->frame_arena = arena_create(FRAME_ARENA_SIZE);
    app->process_list_arena = arena_create(PROCESS_LIST_ARENA_SIZE);
    app->scan_result_arena = arena_create(SCAN_RESULT_ARENA_SIZE);
    app->previous_scan_result_arena = arena_create(SCAN_RESULT_ARENA_SIZE);

    // Initialize thread local contexts.
    // TODO: when we multithread, ensure that all threads do this
    ThreadContext *thread_context = thread_ctx_init();
    ASSERT(thread_context);

    result = (app->permanent_arena != 0)
        && (app->frame_arena != 0)
        && (app->process_list_arena != 0)
        && (app->scan_result_arena != 0)
        && (app->previous_scan_result_arena != 0)
        && (thread_context != 0);

    if (result) {
        result = app_addr_table_init(app->permanent_arena, &app->address_table);
    }

    ASSERT(result);

    app_AddressTableEntry entry = {};
    entry.address = 0xDEADBEEF;

    app_addr_table_push(&app->address_table, entry);

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
