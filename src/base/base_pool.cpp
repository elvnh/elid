#include "base_pool.h"

struct PoolNode {
    PoolNode *next;
};

static PoolNode *pool__get_first_node_address(Pool *pool)
{
    PoolNode *result = (PoolNode *)ptr_byte_offset(pool, POOL_HEADER_SIZE);

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
    node_size = MAX(node_size, sizeof(PoolNode));
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
        result->node_count = node_count;
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

    usize size_to_zero_out = pool->current_pool->node_size;

    if (!current_pool) {
        Pool *new_pool = pool_create_untyped(
            pool->backing_arena,
            pool->node_size,
            pool->node_count,
            pool->node_alignment);

        stack_push_nh(pool, new_pool, next, next);
        current_pool = new_pool;

        // We just allocated a new pool which starts zeroed out, except for the pointers
        // in the node headers, so we just need to memset the node headers to zero.
        size_to_zero_out = sizeof(PoolNode);
    }

    result = current_pool->first_free_node;

    UNPOISON(result, current_pool->node_size);

    stack_pop_nh(current_pool, next, first_free_node);

    pool->current_pool = current_pool;

    zero_mem(result, size_to_zero_out);

    return result;
}

void pool_free(Pool *pool, void *ptr)
{
    // TODO: check that allocation actually comes from this pool
    PoolNode *node = (PoolNode *)ptr;
    stack_push_nh(pool, node, next, first_free_node);

    POISON(node, pool->node_size);

    // Set ourselves to the current pool as there is now at least one free node to be
    // found in it.
    pool->current_pool = pool;
}

void pool_reset(Pool *pool)
{
    for (Pool *current = pool; current; current = current->next) {
        usize node_count = current->node_count;
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
        result += p->node_count;
    }

    return result;
}
