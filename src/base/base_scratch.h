#pragma once

struct ScratchArena {
    Arena *arena;

    ~ScratchArena();
};

ScratchArena scratch_begin(void);
