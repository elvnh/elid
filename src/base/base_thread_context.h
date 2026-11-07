#pragma once

#define THREAD_SCRATCH_ARENA_SIZE MB(4) // TODO: reduce the size of this
#define THREAD_PER_SCAN_SCRATCH_ARENA_SIZE MB(8) // TODO: find good size for this

// A set of categories that all scratch arenas fall into. The category should be picked based on how
// large the arena rougly needs to be.
enum ScratchArenaCategory {
    /* Small, general purpose scratch arenas */
    SCRATCH_ARENA_CATEGORY_GENERAL,

    /* Arenas used for reading entire memory regions at once into memory to then scan */
    SCRATCH_ARENA_CATEGORY_PER_SCAN,

    /* Number of categories defined */
    SCRATCH_ARENA_CATEGORY_COUNT,
};

// All thread-local variables should go here.
struct ThreadContext {
    // Chains of scratch arenas.
    Arena *scratch_arena_chains[SCRATCH_ARENA_CATEGORY_COUNT];

    rng_State rng;

    struct wq__ThreadWorkQueue *private_work_queue;

    b32 initialized;
};

ThreadContext  *thread_ctx_init();
void            thread_ctx_destroy();
ThreadContext  *thread_ctx_get();
