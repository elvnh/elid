static void test_pool_basic(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(ptr);
    }

    arena_destroy(arena);
}

static void test_pool_grow(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n * 4; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(ptr);
    }

    arena_destroy(arena);
}

static void test_pool_grow_beyond_arena(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 200;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n * 4; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(ptr);
    }

    arena_destroy(arena);
}

static void test_pool_free_node_count(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n; ++i) {
        pool_alloc(pool, int);
        REQUIRE(pool_get_free_node_count(pool) == n - (i + 1));
    }

    arena_destroy(arena);
}

static void test_pool_free_node_count_grow(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n; ++i) {
        pool_alloc(pool, int);
    }

    REQUIRE(pool_get_free_node_count(pool) == 0);

    pool_alloc(pool, int);
    size_t new_count = pool_get_free_node_count(pool);

    for (size_t i = 0; i < n - 1; ++i) {
        pool_alloc(pool, int);
        REQUIRE(pool_get_free_node_count(pool) == (new_count - (i + 1)));
    }

    arena_destroy(arena);
}

static void test_pool_total_node_count(void)
{
    Arena *arena = arena_create(1024);

    static const size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    REQUIRE(pool_get_total_node_count(pool) == n);

    int *ptrs[n];
    for (size_t i = 0; i < n; ++i) {
        ptrs[i] = pool_alloc(pool, int);

        REQUIRE(pool_get_total_node_count(pool) == n);
    }

    pool_alloc(pool, int);

    size_t new_total_node_count = pool_get_total_node_count(pool);
    REQUIRE(new_total_node_count > n);

    // Check that total node count doesn't change even as we free nodes
    for (size_t i = 0; i < n; ++i){
        pool_free(pool, ptrs[i]);
        REQUIRE(pool_get_total_node_count(pool) == new_total_node_count);
    }

    arena_destroy(arena);
}

static void test_pool_free(void)
{
    Arena *arena = arena_create(1024);

    static const size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    int *ptrs[n];

    for (size_t i = 0; i < n; ++i) {
        ptrs[i] = pool_alloc(pool, int);
    }

    for (size_t i = 0; i < n; ++i) {
        pool_free(pool, ptrs[i]);
        REQUIRE(pool_get_free_node_count(pool) == i + 1);
    }

    // Check that only the same pointers are acquired when allocating.
    for (size_t i = 0; i < n; ++i) {
        int *ptr = pool_alloc(pool, int);

        bool found = false;

        for (size_t j = 0; j < n; ++j) {
            if (ptrs[j] == ptr) {
                found = true;
                break;
            }
        }

        REQUIRE(found);
    }

    arena_destroy(arena);
}

static void test_pool_used_node_count(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n; ++i) {
        pool_alloc(pool, int);
        REQUIRE(pool_get_used_node_count(pool) == (i + 1));
    }

    arena_destroy(arena);
}

static void test_pool_used_node_count_grow(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 200;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n * 4; ++i) {
        pool_alloc(pool, int);
        REQUIRE(pool_get_used_node_count(pool) == (i + 1));
    }

    arena_destroy(arena);
}

static void test_pool_alignment(void)
{
    Arena *arena = arena_create(1024*1024);

    for (size_t i = 0; i < 8; ++i) {
        size_t alignment = 1u << i;

        Pool *pool = pool_create_untyped(arena, 16, 8, alignment);

        for (size_t repeat = 0; repeat < 128; ++repeat) {
            void *ptr = pool_alloc_untyped(pool);
            REQUIRE(ptr_is_aligned(ptr, alignment));
        }
    }

    arena_destroy(arena);
}

static void test_pool_different_alignments_and_sizes(void)
{
    Arena *arena = arena_create(1024*1024*32);

    size_t n = 32;

    for (size_t i = 0; i < 8; ++i) {
        size_t alignment = 1u << i;

        for (size_t node_size = 1; node_size <= n; ++node_size) {
            for (size_t node_count = 1; node_count <= n; ++node_count) {
                Pool *pool = pool_create_untyped(arena, node_size, node_count, alignment);

                for (size_t repeat = 0; repeat < 64; ++repeat) {
                    void *ptr = pool_alloc_untyped(pool);
                    REQUIRE(ptr_is_aligned(ptr, alignment));

                    // Check that allocation is zeroed
                    for (size_t k = 0; k < node_size; ++k) {
                        REQUIRE(((uint8_t *)ptr)[k] == 0);
                    }
                }
            }
        }

    }

    arena_destroy(arena);
}

static void test_pool_reset(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    pool_alloc(pool, int);
    REQUIRE(pool_get_free_node_count(pool) == 3);

    pool_reset(pool);

    REQUIRE(pool_get_free_node_count(pool) == 4);

    arena_destroy(arena);
}

static void test_pool_reset_after_growing(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n * 4; ++i) {
        pool_alloc(pool, int);
    }

    pool_reset(pool);

    REQUIRE(pool_get_free_node_count(pool) == pool_get_total_node_count(pool));
    REQUIRE(pool_get_free_node_count(pool) == pool_get_free_node_count(pool));

    arena_destroy(arena);
}

static void test_pool_realloc_after_reset_after_growing(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 1;
    size_t n2 = 2;

    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n2; ++i) {
        pool_alloc(pool, int);
    }

    size_t new_total_node_count = pool_get_total_node_count(pool);

    pool_reset(pool);

    REQUIRE(pool_get_free_node_count(pool) == new_total_node_count);

    // Check that reallocing same amount after resetting doesn't cause pool to grow again.
    for (size_t i = 0; i < n2; ++i) {
        pool_alloc(pool, int);
    }

    REQUIRE(pool_get_total_node_count(pool) == new_total_node_count);

    arena_destroy(arena);
}


static void test_pool_zeroed(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(*ptr == 0);
    }

    arena_destroy(arena);
}

static void test_pool_zeroed_after_reset(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n; ++i) {
        int *ptr = pool_alloc(pool, int);
        *ptr = 12345;
    }

    pool_reset(pool);

    for (size_t i = 0; i < n; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(*ptr == 0);
    }

    arena_destroy(arena);
}

static void test_pool_zeroed_after_growing(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n * 4; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(*ptr == 0);
    }

    arena_destroy(arena);
}

static void test_pool_zeroed_after_growing_and_reset(void)
{
    Arena *arena = arena_create(1024);

    size_t n = 4;
    size_t n2 = n * 4;

    Pool *pool = pool_create(arena, int, n);

    for (size_t i = 0; i < n2; ++i) {
        int *ptr = pool_alloc(pool, int);
        *ptr = 12345;
    }

    pool_reset(pool);

    for (size_t i = 0; i < n2; ++i) {
        int *ptr = pool_alloc(pool, int);
        REQUIRE(*ptr == 0);
    }

    arena_destroy(arena);
}
