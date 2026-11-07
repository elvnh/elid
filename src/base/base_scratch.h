#pragma once

// An arena that is automatically released back to a chain of arenas on scope exit.
struct ScratchArena {
    // The category from which the arena was taken.
    ScratchArenaCategory category;

    // The arena used for scratch space.
    Arena *arena;

    ~ScratchArena();
};

// The common case of using general-purpose scratch arenas as backing arena chain.
ScratchArena scratch_begin();

// The general case of using the provided chain of arenas.
ScratchArena scratch_begin_using(ScratchArenaCategory category);
