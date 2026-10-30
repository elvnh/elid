/*
  TODO:
  - Use scratch arenas in tests instead of manually creating and destroying
  - Flatten match list when checking results
  - Unit test next scan
 */

#define TEST_SCAN_ARENA_SIZE MB(8)

#define test_scan_local_array_first(arena, data, count, cmp)            \
    scan_local_memory_first((arena), (data), sizeof(*(data)) * (count), (uintptr_t)(data), (cmp), ALIGNOF_VALUE(*(data)))

#define test_scan_local_array_next(arena, data, count, cmp, prev_matches) \
    scan_local_memory_next((arena), (data), sizeof(*(data)) * (count), (cmp), (prev_matches))

#define test_scan_local_memory_first(arena, data, size, cmp, align)     \
    scan_local_memory_first((arena), (data), (size), (uintptr_t)(data), (cmp), (align))

TEST_CASE(scan_local_first_basic)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 value = 123;

    scan_MatchList *matches = test_scan_local_array_first(
        arena, &value, 1, scan_cmp_make_exact(scan_val_make_int32((u32)value)));
    REQUIRE(matches->total_match_count == 1);
    REQUIRE((s32)matches->match_data[0].value.int32 == value);
    REQUIRE(matches->match_data[0].offset_from_base == 0);

    matches = test_scan_local_array_first(
        arena, &value, 1, scan_cmp_make_exact(scan_val_make_int32(5)));
    REQUIRE(matches->total_match_count == 0);

    arena_destroy(arena);
}

TEST_CASE(scan_local_next_basic)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 value = 123;

    scan_MatchList *matches = test_scan_local_array_first(
        arena, &value, 1, scan_cmp_make_exact(scan_val_make_int32((u32)value)));

    matches = test_scan_local_array_next(
        arena,
        &value,
        1,
        scan_cmp_make_exact(scan_val_make_int32((u32)value)),
        matches);
    REQUIRE(matches->total_match_count == 1);

    arena_destroy(arena);
}

TEST_CASE(scan_local_first_array)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 scan_value = 123;
    s32 values[] = {0, 2, scan_value, 1, scan_value};

    scan_MatchList *matches = test_scan_local_array_first(
        arena, values, array_capacity(values), scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)));

    REQUIRE(matches->total_match_count == 2);
    REQUIRE((s32)matches->match_data[0].value.int32 == scan_value);
    REQUIRE((s32)matches->match_data[1].value.int32 == scan_value);
    REQUIRE(matches->match_data[0].offset_from_base == (2 * sizeof(s32)));
    REQUIRE(matches->match_data[1].offset_from_base == (4 * sizeof(s32)));

    arena_destroy(arena);
}

TEST_CASE(scan_local_next_array)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 scan_value = 123;
    s32 values[] = {0, 2, scan_value, 1, scan_value};

    scan_MatchList *matches = test_scan_local_array_first(
        arena, values, array_capacity(values), scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)));
    usize original_match_count = matches->total_match_count;

    // Changing a non-match shouldn't make a difference.
    values[0] = 123;
    matches = test_scan_local_array_next(
        arena,
        values,
        array_capacity(values),
        scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)),
        matches);
    REQUIRE(matches->total_match_count == original_match_count);

    // Changing a match should exclude it from the next scan.
    values[2] = 50;
    matches = test_scan_local_array_next(
        arena,
        values,
        array_capacity(values),
        scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)),
        matches);
    REQUIRE(matches->total_match_count == (original_match_count - 1));

    values[4] = 1;
    matches = test_scan_local_array_next(
        arena,
        values,
        array_capacity(values),
        scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)),
        matches);
    REQUIRE(matches->total_match_count == 0);

    arena_destroy(arena);
}

TEST_CASE(scan_local_next_different_array)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 scan_value = 123;
    s32 values[] = {0, 2, scan_value, 1, scan_value};

    scan_MatchList *matches = test_scan_local_array_first(
        arena, values, array_capacity(values), scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)));

    s32 values2[array_capacity(values)] = {};
    memcpy(values2, values, sizeof(values));
    matches = test_scan_local_array_next(
        arena,
        values2,
        array_capacity(values2),
        scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)),
        matches);

    REQUIRE(matches->total_match_count == 2);

    arena_destroy(arena);
}

TEST_CASE(scan_local_first_unaligned)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 scan_value = 123;

    char *memory = (char *)arena_alloc_bytes(arena, 1024, 1, alignof(s32));
    memset(memory, 0xCD, 1024);

    // Copy value to unaligned addresses
    memcpy(memory + 1, &scan_value, sizeof(scan_value));
    memcpy(memory + 7, &scan_value, sizeof(scan_value));
    memcpy(memory + 19, &scan_value, sizeof(scan_value));
    memcpy(memory + 987, &scan_value, sizeof(scan_value));

    // Copy value to aligned addresses
    memcpy(memory + 12, &scan_value, sizeof(scan_value));
    memcpy(memory + 24, &scan_value, sizeof(scan_value));
    memcpy(memory + 788, &scan_value, sizeof(scan_value));
    memcpy(memory + 1020, &scan_value, sizeof(scan_value));

    scan_MatchList *matches = test_scan_local_memory_first(
        arena, memory, 1024, scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)), 1);

    REQUIRE(matches->total_match_count == 8);

    arena_destroy(arena);
}

TEST_CASE(scan_local_first_aligned)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 scan_value = 123;

    char *memory = (char *)arena_alloc_bytes(arena, 1024, 1, alignof(s32));
    memset(memory, 0xCD, 1024);

    // Copy value to unaligned addresses
    memcpy(memory + 1, &scan_value, sizeof(scan_value));
    memcpy(memory + 7, &scan_value, sizeof(scan_value));
    memcpy(memory + 19, &scan_value, sizeof(scan_value));
    memcpy(memory + 987, &scan_value, sizeof(scan_value));

    // Copy value to aligned addresses
    memcpy(memory + 12, &scan_value, sizeof(scan_value));
    memcpy(memory + 24, &scan_value, sizeof(scan_value));
    memcpy(memory + 788, &scan_value, sizeof(scan_value));
    memcpy(memory + 1020, &scan_value, sizeof(scan_value));

    scan_MatchList *matches = test_scan_local_memory_first(
        arena, memory, 1024, scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)), 4);

    // Only the aligned values should count.
    REQUIRE(matches->total_match_count == 4);

    scan_ResultView view = scan_match_list_make_view(arena, matches);
    REQUIRE(view.count == 4);
    REQUIRE(view.data[0].match->offset_from_base == 12);
    REQUIRE(view.data[0].match->offset_from_base == 12);
    REQUIRE(view.data[1].match->offset_from_base == 24);
    REQUIRE(view.data[2].match->offset_from_base == 788);
    REQUIRE(view.data[3].match->offset_from_base == 1020);

    arena_destroy(arena);
}

TEST_CASE(scan_local_first_repeating_unaligned_values)
{
    Arena *arena = arena_create(TEST_SCAN_ARENA_SIZE);
    s32 scan_value = (s32)0xF00BF00B; // Scan value consists of two repeating 2-byte values

    s32 values[] = {scan_value, scan_value, scan_value, 0};

    // If we only scan every 4th address from the base, it should find all the values.
    scan_MatchList *matches = scan_local_memory_first(
        arena,
        values,
        array_capacity(values) * sizeof(*values),
        (uintptr_t)values,
        scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)),
        4);

    REQUIRE(matches->total_match_count == 3);

    // If we only scan every 2nd address from the base, every instance of the value should give
    // two matches except for the last one.
    matches = scan_local_memory_first(
        arena,
        values,
        array_capacity(values) * sizeof(*values),
        (uintptr_t)values,
        scan_cmp_make_exact(scan_val_make_int32((u32)scan_value)),
        2);

    REQUIRE(matches->total_match_count == 5);


    arena_destroy(arena);
}
