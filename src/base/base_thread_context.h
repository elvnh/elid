#pragma once

#define THREAD_SCRATCH_ARENA_SIZE MB(4)

// NOTE: This is only used to store a regions memory at a time,
// so it doesn't need to be too large
#define THREAD_SCAN_SCRATCH_ARENA_SIZE MB(32)

// All thread-local variables should go here.
struct ThreadContext {
    // Chain of scratch arenas.
    Arena *first_scratch_arena;

    // For reading memory to scan and then discard.
    Arena *per_scan_scratch_arena;

    b32 initialized;
};

ThreadContext  *thread_ctx_init(void);
ThreadContext  *thread_ctx_get(void);
