#pragma once

#define THREAD_SCRATCH_ARENA_SIZE MB(4)

// NOTE: This is only used to store a regions memory at a time,
// so it doesn't need to be too large
#define THREAD_SCAN_SCRATCH_ARENA_SIZE MB(32)

struct ThreadContext {
    // For function-local allocations. Should usually be used with scratch_begin().
    Arena *scratch_arena;
    // For reading memory to scan and then discard.
    Arena *per_scan_scratch_arena;
};

// TODO: make sure to call this when creating a new thread
ThreadContext  *thread_ctx_init(void);
ThreadContext  *thread_ctx_get(void);
Temp            scratch_begin(void);
void            scratch_end(Temp *temp);
