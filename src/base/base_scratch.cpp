#include "base_scratch.h"

static Arena **scratch__get_arena_chain_head(ScratchArenaCategory category)
{
    ASSERT(category >= 0);
    ASSERT(category < SCRATCH_ARENA_CATEGORY_COUNT);

    Arena **result = &thread_ctx_get()->scratch_arena_chains[category];

    return result;
}

ScratchArena scratch_begin()
{
    return scratch_begin_using(SCRATCH_ARENA_CATEGORY_GENERAL);
}

ScratchArena scratch_begin_using(ScratchArenaCategory category)
{
    // Pop the first scratch arena of the linked list.
    Arena **arena_list_head = scratch__get_arena_chain_head(category);
    Arena *arena = *arena_list_head;

    if (!arena) {
        arena = arena_create(THREAD_SCRATCH_ARENA_SIZE);
    }

    *arena_list_head = arena->next;

    arena->next = 0;
    arena->current_arena = arena;

    arena_reset(arena);

    return {category, arena};
}


ScratchArena::~ScratchArena()
{
    ASSERT(arena);
    // Prepend this scratch arena into the thread local list of scratch arenas.
    Arena **arena_list_head_ptr = scratch__get_arena_chain_head(category);

    // Find the last node in this chain of arenas, in case the arena grew after obtaining it.
    Arena *last_arena = arena;

    while (last_arena->next) {
        last_arena = last_arena->next;
    }

    ASSERT(last_arena);
    ASSERT(!last_arena->next);

    last_arena->next = *arena_list_head_ptr;

    *arena_list_head_ptr = arena;
}
