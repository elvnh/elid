#pragma once

// TODO: reduce the size of this, use other chain of arenas for scan scratch arenas
#define THREAD_SCRATCH_ARENA_SIZE MB(4)

// All thread-local variables should go here.
struct ThreadContext {
    // Chain of scratch arenas.
    Arena *first_scratch_arena;

    rng_State rng;

    struct wq__ThreadWorkQueue *private_work_queue;

    b32 initialized;
};

ThreadContext  *thread_ctx_init();
void            thread_ctx_destroy();
ThreadContext  *thread_ctx_get();
