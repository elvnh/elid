#include "base_thread_context.h"

// TODO: typedef for thread ID
thread_global ThreadContext g__thread_context;
global u64 g__next_thread_id;

ThreadContext *thread_ctx_init()
{
    if (ALWAYS(!g__thread_context.initialized)) {
        g__thread_context.first_scratch_arena = arena_create(THREAD_SCRATCH_ARENA_SIZE);
        g__thread_context.per_scan_scratch_arena = arena_create(THREAD_SCAN_SCRATCH_ARENA_SIZE);

        rng_seed(&g__thread_context.rng, get_monotonic_clock_ms());

        g__thread_context.id = atomic_inc_post_u64(&g__next_thread_id);

        g__thread_context.initialized = true;
    }

    ThreadContext *result = thread_ctx_get();

    return result;
}

void thread_ctx_destroy()
{
    ThreadContext *ctx = thread_ctx_get();
    ASSERT(ctx);

    if (ALWAYS(ctx->initialized)) {
        arena_destroy(ctx->first_scratch_arena);
        arena_destroy(ctx->per_scan_scratch_arena);
    }
}

ThreadContext *thread_ctx_get()
{
    ThreadContext *result = 0;

    if (!g__thread_context.initialized) {
        thread_ctx_init();
    }

    result = &g__thread_context;

    ASSERT(g__thread_context.initialized);

    return result;
}

u64 thread_ctx_get_id()
{
    u64 result = thread_ctx_get()->id;

    return result;
}
