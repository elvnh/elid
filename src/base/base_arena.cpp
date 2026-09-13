#include "base_arena.h"

struct ArenaBuffer {
    Arena *data;
    usize size_excluding_header;
};

static ArenaBuffer arena__alloc_new_buffer(usize min_size)
{
    ArenaBuffer result = {};

    usize size_plus_header = 0;

    if (safe_add_usize(min_size, ARENA_HEADER_SIZE, &size_plus_header)) {
        // Round up to nearest page size.
        usize page_size = get_system_page_size();
        usize total_allocation_size = align_pow2(size_plus_header, page_size);
        usize usable_size = total_allocation_size - ARENA_HEADER_SIZE;

        result.data = (Arena *)allocate_memory(total_allocation_size);
        result.size_excluding_header = usable_size;
    }

    return result;
}

static void *arena__get_pointer_to_offset(Arena *arena, usize offset)
{
    ASSERT(offset < arena->size_excluding_header);
    void *result = (u8 *)arena + ARENA_HEADER_SIZE + offset;

    return result;
}

Arena *arena_create(usize size)
{
    ASSERT(size > 0);
    ASSERT(sizeof(Arena) <= ARENA_HEADER_SIZE);

    Arena *result = 0;
    ArenaBuffer new_buffer = arena__alloc_new_buffer(size);

    if (new_buffer.data) {
        // NOTE: Memory is already zeroed here.
        result = new_buffer.data;
        result->current_arena = result;
        result->size_excluding_header = new_buffer.size_excluding_header;

        POISON(arena__get_pointer_to_offset(result, 0), new_buffer.size_excluding_header);
    }

    return result;
}

void *arena_alloc_bytes(Arena *arena, usize item_count, usize item_size, usize alignment)
{
    ASSERT(item_count > 0);
    ASSERT(item_size > 0);
    ASSERT(alignment > 0);
    ASSERT(is_pow2(alignment));
    ASSERT(alignment <= ARENA_HEADER_SIZE);

    void *result = 0;
    Arena *current_arena = arena->current_arena;

    usize byte_count = 0;
    usize size_to_zero_out = 0;

    if (safe_mul_usize(item_count, item_size, &byte_count)) {
        usize aligned_offset = align_pow2(current_arena->offset, alignment);
        usize next_offset = 0;

        size_to_zero_out = byte_count;

        b32 allocation_fits =
            safe_add_usize(aligned_offset, byte_count, &next_offset)
            && (next_offset <= current_arena->size_excluding_header);

        if (allocation_fits) {
            result = arena__get_pointer_to_offset(current_arena, aligned_offset);

            current_arena->offset = next_offset;
        } else {
            // The allocation didn't fit in the current arena, walk the rest of
            // the chain of arenas to try to find one that does.
            current_arena = current_arena->next;

            while (current_arena && (byte_count > current_arena->size_excluding_header)) {
                current_arena = current_arena->next;
            }

            if (!current_arena) {
                // No arena that fit the allocation was found, allocate a new one.
                current_arena = arena_create(byte_count);
                stack_push_nh_no_count(arena->current_arena, current_arena, next, next);

                // No need to clear the allocation as we just allocated a new arena which
                // starts out cleared to zero.
                size_to_zero_out = 0;
            }

            // We switched to a new arena so we'll always start at offset 0.
            result = arena__get_pointer_to_offset(current_arena, 0);
            current_arena->offset = byte_count;
        }
    }

    if (result) {
        ASSERT(byte_count > 0);
        UNPOISON(result, byte_count);

        zero_mem(result, size_to_zero_out);
    }

    arena->current_arena = current_arena;

    ASSERT(is_aligned((usize)result, alignment));

    return result;
}

void arena_pop_to(Arena *arena, usize offset)
{
    Arena *current_arena = arena;

    usize current_offset = 0;

    while (current_arena) {
        usize next_offset = current_offset + current_arena->size_excluding_header;

        if ((offset >= current_offset) && (offset < next_offset)) {
            break;
        }

        POISON(arena__get_pointer_to_offset(arena, 0), current_arena->size_excluding_header);

        current_arena = current_arena->next;
        current_offset = next_offset;
    }

    ASSERT(current_arena);

    usize relative_offset = offset - current_offset;
    current_arena->offset = relative_offset;

    POISON(
        arena__get_pointer_to_offset(current_arena, relative_offset),
        current_arena->size_excluding_header - relative_offset);

    arena->current_arena = current_arena;
}

void arena_reset(Arena *arena)
{
    arena_pop_to(arena, 0);
}

void arena_destroy(Arena *arena)
{
    Arena *current_arena = arena;

    while (current_arena) {
        Arena *next = current_arena->next;

        usize total_allocation_size = ARENA_HEADER_SIZE + current_arena->size_excluding_header;
        free_memory(current_arena, total_allocation_size);

        current_arena = next;
    }
}

usize arena_get_memory_usage(Arena *arena)
{
    usize result = 0;

    Arena *it = arena;

    while (it != arena->current_arena) {
        result += it->size_excluding_header;

        it = it->next;
    }

    result += arena->current_arena->offset;

    return result;
}

Temp temp_begin(Arena *arena)
{
    Temp result = {};
    result.arena = arena;
    result.start_offset = arena_get_memory_usage(arena);

    return result;
}

void temp_end(Temp *temp)
{
    arena_pop_to(temp->arena, temp->start_offset);

    temp->arena = 0;
    temp->start_offset = 0;
}

Temp::~Temp()
{
    temp_end(this);
}
