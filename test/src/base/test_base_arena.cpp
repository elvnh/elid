// TODO: remove test prefix from these
TEST_CASE(arena_basic)
{
    Arena *arena = arena_create(1024);

    int *a = arena_alloc_struct(arena, int);
    int *b = arena_alloc_struct(arena, int);

    REQUIRE(a != b);
    REQUIRE(a < b);

    arena_destroy(arena);
}

TEST_CASE(arena_reset)
{
    Arena *arena = arena_create(1024);

    int *a = arena_alloc_struct(arena, int);
    arena_reset(arena);
    int *b = arena_alloc_struct(arena, int);

    REQUIRE(a == b);

    arena_destroy(arena);
}

TEST_CASE(arena_reset_after_growing)
{
    Arena *arena = arena_create(1024);

    int *a = arena_alloc_struct(arena, int);
    arena_alloc_array(arena, int, 2000);
    arena_reset(arena);
    int *b = arena_alloc_struct(arena, int);

    REQUIRE(a == b);

    arena_destroy(arena);
}

TEST_CASE(arena_memory_usage_basic)
{
    Arena *arena = arena_create(1024);
    REQUIRE(arena_get_memory_usage(arena) == 0);

    int *a = arena_alloc_struct(arena, int);
    REQUIRE(arena_get_memory_usage(arena) >= sizeof(*a));

    arena_reset(arena);
    REQUIRE(arena_get_memory_usage(arena) == 0);

    arena_destroy(arena);
}

TEST_CASE(arena_memory_usage_after_growing)
{
    Arena *arena = arena_create(1024);

    arena_alloc_array(arena, int, 1000);
    REQUIRE(arena_get_memory_usage(arena) >= sizeof(int) * 1000);

    arena_reset(arena);
    REQUIRE(arena_get_memory_usage(arena) == 0);

    arena_alloc_struct(arena, int);
    REQUIRE(arena_get_memory_usage(arena) >= sizeof(int));

    arena_destroy(arena);
}

TEST_CASE(arena_zeroed_fresh_arena)
{
    Arena *arena = arena_create(1024);

    for (int i = 0; i < 10; ++i) {
        int *a = arena_alloc_struct(arena, int);
        REQUIRE(*a == 0);
    }

    arena_destroy(arena);
}

TEST_CASE(arena_zeroed_after_reset)
{
    Arena *arena = arena_create(1024);

    int *a = arena_alloc_struct(arena, int);
    *a = 123;

    arena_reset(arena);
    a = arena_alloc_struct(arena, int);
    REQUIRE(*a == 0);

    arena_destroy(arena);
}

TEST_CASE(arena_zeroed_after_growing_and_resetting)
{
    Arena *arena = arena_create(1024);

    uint32_t n = 4096;

    for (uint32_t i = 0; i < n; ++i) {
        char *c = arena_alloc_struct(arena, char);
        *c = 123;
    }

    arena_reset(arena);

    for (uint32_t i = 0; i < n; ++i) {
        char *c = arena_alloc_struct(arena, char);
        REQUIRE(*c == 0);
    }

    arena_destroy(arena);
}

TEST_CASE(arena_alignment)
{
    Arena *arena = arena_create(1024);

    for (uint32_t i = 0; i < 8; ++i) {
        size_t alignment = 1u << i;
        void *ptr = arena_alloc_bytes(arena, 1, 1, alignment);
        REQUIRE(ptr_is_aligned(ptr, alignment));
    }

    arena_destroy(arena);

}
TEST_CASE(arena_alignment_after_growing)
{
    Arena *arena = arena_create(1024);

    arena_alloc_array(arena, int, 2000);

    for (uint32_t i = 0; i < 8; ++i) {
        size_t alignment = 1u << i;
        void *ptr = arena_alloc_bytes(arena, 1, 1, alignment);
        REQUIRE(ptr_is_aligned(ptr, alignment));
    }

    arena_destroy(arena);
}

TEST_CASE(arena_alignment_after_growing_and_resetting)
{
    Arena *arena = arena_create(1024);

    arena_alloc_array(arena, int, 2000);
    arena_reset(arena);

    for (uint32_t i = 0; i < 8; ++i) {
        size_t alignment = 1u << i;
        void *ptr = arena_alloc_bytes(arena, 1000, 10, alignment);
        REQUIRE(ptr_is_aligned(ptr, alignment));
    }

    arena_destroy(arena);
}

TEST_CASE(arena_pop_to)
{
    Arena *arena = arena_create(1024);

    arena_alloc_array(arena, char, 100);

    arena_pop_to(arena, 50);

    REQUIRE(arena_get_memory_usage(arena) == 50);

    arena_destroy(arena);
}

TEST_CASE(arena_pop_to_after_growing)
{
    Arena *arena = arena_create(1024);

    arena_alloc_array(arena, char, 10000);

    arena_pop_to(arena, 50);

    REQUIRE(arena_get_memory_usage(arena) == 50);

    arena_destroy(arena);
}

TEST_CASE(arena_pop_to_new_buffer)
{
    Arena *arena = arena_create(1024);

    arena_alloc_array(arena, char, 10000);

    arena_pop_to(arena, 5000);

    REQUIRE(arena_get_memory_usage(arena) == 5000);

    arena_destroy(arena);
}

TEST_CASE(arena_different_alignments_and_sizes)
{
    Arena *arena = arena_create(1024);

    // Repeat multiple times so arena has a chance to grow
    for (uint32_t i = 0; i < 16; ++i) {
        for (uint32_t p = 0; p < 8; ++p) {
            size_t alignment = 1u << p;

            for (uint32_t size = 1; size <= 1024; ++size) {
                void *ptr = arena_alloc_bytes(arena, 1, size, alignment);

                REQUIRE(ptr);
                REQUIRE(ptr_is_aligned(ptr, alignment));

                // Check that allocation is zeroed out
                for (size_t k = 0; k < size; ++k) {
                    REQUIRE(((uint8_t *)ptr)[k] == 0);
                }
            }
        }
    }

    arena_destroy(arena);
}

TEST_CASE(arena_allocate_entire_arena)
{
    size_t size = 1024;
    Arena *arena = arena_create(size);

    void *ptr = arena_alloc_bytes(arena, size, 1, 1);
    REQUIRE(ptr);

    REQUIRE(arena_get_memory_usage(arena) == size);

    arena_destroy(arena);
}

TEST_CASE(arena_allocate_entire_arena_different_sizes)
{
    size_t sizes[] = {1, 1023, 4096, 5000, 10000, 65535, 65536, 70000};

    for (size_t i = 0; i < array_capacity(sizes); ++i) {
        size_t size = sizes[i];

        Arena *arena = arena_create(size);

        void *ptr = arena_alloc_bytes(arena, size, 1, 1);
        REQUIRE(ptr);

        REQUIRE(arena_get_memory_usage(arena) == size);

        arena_destroy(arena);
    }

}
