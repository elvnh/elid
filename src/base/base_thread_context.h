#pragma once

#define THREAD_SCRATCH_ARENA_SIZE MB(4)

// NOTE: This is only used to store a regions memory at a time,
// so it doesn't need to be too large
#define THREAD_SCAN_SCRATCH_ARENA_SIZE MB(32)

// All thread-local variables should go here.
struct ThreadContext {
    u64 id;

    // Chain of scratch arenas.
    Arena *first_scratch_arena;

    // For reading memory to scan and then discard.
    Arena *per_scan_scratch_arena; // TODO: move into scan_RemoteScanArgs

    rng_State rng;

    struct wq__ThreadWorkQueue *private_work_queue;

    b32 initialized;
};

ThreadContext  *thread_ctx_init();
void            thread_ctx_destroy();
ThreadContext  *thread_ctx_get();
u64             thread_ctx_get_id();
