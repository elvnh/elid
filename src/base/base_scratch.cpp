#include "base_scratch.h"

ScratchArena scratch_begin(void)
{
    // Pop the first scratch arena of the linked list.

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
    // Prepend this scratch arena into the thread local list of scratch arenas.

    ASSERT(arena);

    // Find the last node in this chain of arenas, in case the arena grew after obtaining it.
    Arena *last_arena = arena;

    while (last_arena->next) {
        last_arena = last_arena->next;
    }

    ASSERT(last_arena);
    ASSERT(!last_arena->next);

    last_arena->next = thread_ctx_get()->first_scratch_arena;

    thread_ctx_get()->first_scratch_arena = arena;
}
