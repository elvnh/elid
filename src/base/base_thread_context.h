#pragma once

#define THREAD_SCRATCH_ARENA_SIZE MB(4)

struct ThreadContext {
    Arena *scratch_arena;
};

// TODO: make sure to call this when creating a new thread
ThreadContext  *thread_ctx_init(void);
ThreadContext  *thread_ctx_get(void);
Temp            scratch_begin(void);
void            scratch_end(Temp *temp);
