#include "base_pool.h"

// TODO: rename
struct PoolNode {
    PoolNode *next;
    PoolNodeGeneration generation;
};

static usize pool__node_header_size(usize node_alignment)
{
    usize result = align_pow2(sizeof(PoolNode), node_alignment);

    return result;
}

static void *pool__node_user_ptr_from_header(Pool *pool, PoolNode *node)
{
    void *result = ptr_byte_offset(node, pool__node_header_size(pool->node_alignment));

    return result;
}

static PoolNode *pool__header_from_user_ptr(Pool *pool, void *ptr)
{
    ssize header_size = (ssize)pool__node_header_size(pool->node_alignment);
    PoolNode *result = (PoolNode *)ptr_byte_offset(ptr, -header_size);

    return result;
}

static PoolNode *pool__get_first_node_address(Pool *pool)
{
    PoolNode *result = (PoolNode *)ptr_byte_offset(pool, POOL_HEADER_SIZE);

    return result;
}

static usize pool__get_usable_node_size(Pool *pool)
{
    usize result =
        pool->node_size - pool__node_header_size(pool->node_alignment);

    return result;
}

Pool *pool_create_untyped(Arena *optional_arena, usize node_size, usize node_count, usize node_alignment)
{
    ASSERT(node_size > 0);
    ASSERT(node_count > 0);
    ASSERT(node_alignment > 0);
    ASSERT(is_pow2(node_alignment));
    ASSERT(node_alignment <= POOL_HEADER_SIZE);

    Pool *result = 0;

    Arena *arena = optional_arena;

    node_alignment = MAX(node_alignment, alignof(PoolNode));

    usize header_size = pool__node_header_size(node_alignment);
    node_size = header_size + node_size;
    node_size = align_pow2(node_size, node_alignment);

    usize byte_count = 0;

    if (safe_mul_usize(node_size, node_count, &byte_count)) {
        usize byte_count_including_header = byte_count + POOL_HEADER_SIZE;

        if (!arena) {
            arena = arena_create(byte_count_including_header);
        }

        result = (Pool *)arena_alloc_bytes(arena, byte_count_including_header, 1, POOL_HEADER_SIZE);
        result->current_pool = result;
        result->backing_arena = arena;
        result->node_size = node_size;
        result->node_capacity = node_count;
        result->node_alignment = node_alignment;

        pool_reset(result);
    }

    ASSERT(is_aligned((usize)result->first_free_node, node_alignment));

    return result;
}

void *pool_alloc_untyped(Pool *pool)
{
    void *result = 0;

    Pool *current_pool = pool->current_pool;

    while (current_pool && !current_pool->first_free_node) {
        current_pool = current_pool->next;
    }

    usize size_to_zero_out = 0;

    if (!current_pool) {
        Pool *new_pool = pool_create_untyped(
            pool->backing_arena,
            pool->node_size,
            pool->node_capacity,
            pool->node_alignment);

        stack_push_nh_no_count(pool, new_pool, next, next);
        current_pool = new_pool;

        // We just allocated a new pool which starts zeroed out, no need to zero anything.
    } else {
        // We only want to zero out the area of the node user by the user, not the header.
        size_to_zero_out = pool__get_usable_node_size(current_pool);

    }

    PoolNode *node = current_pool->first_free_node;
    result = pool__node_user_ptr_from_header(current_pool, node);

    UNPOISON(result, pool__get_usable_node_size(current_pool));

    stack_pop_nh_no_count(current_pool, next, first_free_node);

    pool->current_pool = current_pool;

    if (size_to_zero_out > 0) {
        zero_mem(result, size_to_zero_out);
    }

    return result;
}

void pool_free(Pool *pool, void *ptr)
{
    // TODO: check that allocation actually comes from this pool
    PoolNode *node = pool__header_from_user_ptr(pool, ptr);
    stack_push_nh_no_count(pool, node, next, first_free_node);

    // Generation is bumped each time the node is recycled.
    ++node->generation;

    POISON(node, pool->node_size);

    // Set ourselves to the current pool as there is now at least one free node to be
    // found in it.
    pool->current_pool = pool;
}

void pool_reset(Pool *pool)
{
    for (Pool *current = pool; current; current = current->next) {
        usize node_count = current->node_capacity;
        usize node_size = current->node_size;

        UNPOISON(pool__get_first_node_address(current), node_count * node_size);

        current->first_free_node = pool__get_first_node_address(current);

        for (usize i = 0; i < node_count; ++i) {
            PoolNode *node = ptr_byte_offset(current->first_free_node, i * node_size);
            PoolNode *next = 0;

            ASSERT(is_aligned((usize)node, current->node_alignment));

            if (i < (node_count - 1)) {
                next = ptr_byte_offset(current->first_free_node, (i + 1) * node_size);
            }

            node->next = next;
        }

        POISON(pool__get_first_node_address(current), node_count * node_size);
    }

    pool->current_pool = pool;
}

usize pool_get_free_node_count(Pool *pool)
{
    usize count = 0;

    for (Pool *p = pool; p; p = p->next) {
        PoolNode *node = p->first_free_node;

        while (node) {
            UNPOISON(node, p->node_size);
            PoolNode *next = node->next;
            POISON(node, p->node_size);

            node = next;

            ++count;
        }
    }

    return count;
}

usize pool_get_used_node_count(Pool *pool)
{
    usize total_node_count = pool_get_total_node_count(pool);
    usize free_node_count = pool_get_free_node_count(pool);
    usize result = total_node_count - free_node_count;

    return result;
}

usize pool_get_total_node_count(Pool *pool)
{
    usize result = 0;

    for (Pool *p = pool; p; p = p->next) {
        result += p->node_capacity;
    }

    return result;
}

PoolNodeGeneration pool_node_generation(Pool *pool, void *ptr)
{
    PoolNode *node = pool__header_from_user_ptr(pool, ptr);

    PoolNodeGeneration result = node->generation;

    return result;
}
