#include "base_thread_context.h"

thread_global ThreadContext g__thread_context;

ThreadContext *thread_ctx_init()
{
    if (ALWAYS(!g__thread_context.initialized)) {
        g__thread_context.scratch_arena_chains[SCRATCH_ARENA_CATEGORY_GENERAL] = arena_create(THREAD_SCRATCH_ARENA_SIZE);
        g__thread_context.scratch_arena_chains[SCRATCH_ARENA_CATEGORY_PER_SCAN] = arena_create(THREAD_PER_SCAN_SCRATCH_ARENA_SIZE);

        #if DEBUG_BUILD
        {
            for (ScratchArenaCategory cat = {}; cat < SCRATCH_ARENA_CATEGORY_COUNT; inc_enum(&cat)) {
                ASSERT(g__thread_context.scratch_arena_chains[cat] && "Did not initiialize all scratch arena chains");
            }
        }
        #endif

        rng_seed(&g__thread_context.rng, get_monotonic_clock_ms());

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
        for (ScratchArenaCategory cat = {}; cat < SCRATCH_ARENA_CATEGORY_COUNT; inc_enum(&cat)) {
            arena_destroy(ctx->scratch_arena_chains[cat]);
        }
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
