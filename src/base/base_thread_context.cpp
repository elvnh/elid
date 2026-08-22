#include "base_thread_context.h"

thread_global ThreadContext g__thread_context;

ThreadContext *thread_ctx_init(void)
{
    g__thread_context.scratch_arena = arena_create(THREAD_SCRATCH_ARENA_SIZE);
    g__thread_context.per_scan_scratch_arena = arena_create(THREAD_SCAN_SCRATCH_ARENA_SIZE);

    ThreadContext *result = thread_ctx_get();

    return result;
}

ThreadContext *thread_ctx_get(void)
{
    ThreadContext *result = 0;

    b32 is_initialized = (g__thread_context.scratch_arena != 0)
        && (g__thread_context.per_scan_scratch_arena != 0);

    if (ALWAYS(is_initialized)) {
        result = &g__thread_context;
    }

    return result;
}

Temp scratch_begin(void)
{
    ThreadContext *context = thread_ctx_get();
    ASSERT(context);

    Temp result = temp_begin(context->scratch_arena);

    return result;
}

void scratch_end(Temp *temp)
{
    temp_end(temp);
}
