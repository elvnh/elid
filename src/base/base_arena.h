#pragma once

// TODO: static assert that this is >= sizeof(Arena)
#define ARENA_HEADER_SIZE 128

// TODO: should it be possible to provide own backing buffer?
struct Arena {
    Arena  *current_arena;
    Arena  *next;
    usize   size_excluding_header;
    usize   offset;
};

struct Temp {
    Arena *arena;
    usize  start_offset;

    ~Temp();
};

Arena  *arena_create(usize size);
void   *arena_alloc_bytes(Arena *arena, usize item_count, usize item_size, usize alignment);
void    arena_pop_to(Arena *arena, usize offset);
void    arena_reset(Arena *arena);
void    arena_destroy(Arena *arena);
usize   arena_get_memory_usage(Arena *arena);
#define arena_alloc_struct(arena, type) arena_alloc_array(arena, type, 1)
#define arena_alloc_array(arena, type, count) ((type *)arena_alloc_bytes((arena), (count), sizeof(type), alignof(type)))

Temp    temp_begin(Arena *arena);
void    temp_end(Temp *temp);
