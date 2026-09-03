#pragma once

#define POOL_HEADER_SIZE 128

typedef struct PoolNode PoolNode;

struct Pool {
    Pool      *current_pool;
    Pool      *next;
    Arena     *backing_arena;
    usize      node_size;
    usize      node_capacity;
    usize      node_alignment;
    PoolNode  *first_free_node;
};

//STATIC_ASSERT(sizeof(Pool) <= POOL_HEADER_SIZE);

Pool  *pool_create_untyped(Arena *optional_arena, usize node_size, usize node_count, usize node_alignment);
void  *pool_alloc_untyped(Pool *pool);
void   pool_free(Pool *pool, void *ptr);
void   pool_reset(Pool *pool);
usize  pool_get_free_node_count(Pool *pool);
usize  pool_get_used_node_count(Pool *pool);
usize  pool_get_total_node_count(Pool *pool);

#define pool_create(arena, t, count) pool_create_untyped(arena, sizeof(t), count, alignof(t))
#define pool_alloc(pool, t)                                             \
    (ASSERT_AND_THEN(                                                   \
        (sizeof(t) <= (pool)->node_size) && (alignof(t) <= (pool)->node_alignment), \
        (t *)pool_alloc_untyped((pool))))
