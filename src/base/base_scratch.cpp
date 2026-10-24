#include "base_scratch.h"

ScratchArena scratch_begin(void)
{
    Arena *arena = thread_ctx_get()->first_scratch_arena;

    if (!arena) {
        arena = arena_create(THREAD_SCRATCH_ARENA_SIZE);
    }

    thread_ctx_get()->first_scratch_arena = arena->next;

    arena->next = 0;
    arena->current_arena = arena;

    arena_reset(arena);

    return {arena};
}

ScratchArena::~ScratchArena()
{
    ASSERT(arena);

    Arena *last_arena = arena;

    while (last_arena->next) {
        last_arena = last_arena->next;
    }

    ASSERT(last_arena);
    ASSERT(!last_arena->next);

    last_arena->next = thread_ctx_get()->first_scratch_arena;

    thread_ctx_get()->first_scratch_arena = arena;
}
