#include "app_state.h"

#define FRAME_ARENA_SIZE        MB(8)
#define PROCESS_LIST_ARENA_SIZE MB(4)
#define SCAN_RESULT_ARENA_SIZE  sizeof(scan_Match) * THOUSAND(100)

b32 app_init(app_State *app)
{
    b32 result = false;

    app->frame_arena = arena_create(FRAME_ARENA_SIZE);
    app->process_list_arena = arena_create(PROCESS_LIST_ARENA_SIZE);
    app->scan_result_arena = arena_create(SCAN_RESULT_ARENA_SIZE);

    // Initialize thread local contexts.
    // TODO: when we multithread, ensure that all threads do this
    ThreadContext *thread_context = thread_ctx_init();
    ASSERT(thread_context);

    result = (app->frame_arena != 0)
        && (app->process_list_arena != 0)
        && (app->scan_result_arena != 0)
        && (thread_context != 0);

    ASSERT(result);

    return result;
}
