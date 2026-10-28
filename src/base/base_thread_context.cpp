#include "base_thread_context.h"

thread_global ThreadContext g__thread_context;

ThreadContext *thread_ctx_init(void)
{
    g__thread_context.first_scratch_arena = arena_create(THREAD_SCRATCH_ARENA_SIZE);
    g__thread_context.per_scan_scratch_arena = arena_create(THREAD_SCAN_SCRATCH_ARENA_SIZE);

    rng_seed(&g__thread_context.rng, get_monotonic_clock_ms());

    g__thread_context.initialized = true;

    ThreadContext *result = thread_ctx_get();

    return result;
}

ThreadContext *thread_ctx_get(void)
{
    ThreadContext *result = 0;

    if (!g__thread_context.initialized) {
        thread_ctx_init();
    }

    result = &g__thread_context;

    ASSERT(g__thread_context.initialized);

    return result;
}
