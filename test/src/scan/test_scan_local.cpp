// For searching aligned values in an array.
#define test_scan_local_array_first2(arena, data, count, cmp)           \
    scan_match_list_make_view((arena),                                  \
        scan_local_memory_first((arena), (data), sizeof(*(data)) * (count), (uintptr_t)(data), (cmp), ALIGNOF_VALUE(*(data))))

// For searching values with arbitrary alignment in an array.
#define test_scan_local_memory_first2(arena, data, size, cmp, align)    \
    scan_match_list_make_view((arena),                                  \
        scan_local_memory_first((arena), (data), size, (uintptr_t)(data), (cmp), (align)))

static b32 test_check_local_match(scan_MatchDetails details, scan_TypedValue expected_value, uintptr_t expected_offset)
{
    // Since we created a scan_ResultView from a scan_MatchList, the region the matches are located
    // in are unknown, so retrieving the address will just return an offset.
    uintptr_t offset = scan_address_from_match_details(details);

    b32 result = (offset == expected_offset)
        && scan_val_eq(expected_value.type, expected_value.value, details.match->value);

    return result;
}

TEST_CASE(scan_local_first_single_value_success)
{
    ScratchArena scratch = scratch_begin();

    s32 buffer = 123;

    scan_TypedValue scan_val = scan_val_make_int32((u32)buffer);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first2(scratch.arena, &buffer, 1, cmp);

    REQUIRE(results.count == 1);
    REQUIRE(test_check_local_match(results.data[0], scan_val, 0));
}

TEST_CASE(scan_local_first_single_value_fail)
{
    ScratchArena scratch = scratch_begin();

    s32 buffer = 123;

    scan_TypedValue scan_val = scan_val_make_int32(5);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first2(scratch.arena, &buffer, 1, cmp);

    REQUIRE(results.count == 0);
}

TEST_CASE(scan_local_first_small_array_success)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first2(scratch.arena, values, array_capacity(values), cmp);

    REQUIRE(results.count == 2);
    REQUIRE(test_check_local_match(results.data[0], scan_val, 2 * sizeof(s32)));
    REQUIRE(test_check_local_match(results.data[1], scan_val, 4 * sizeof(s32)));
}

TEST_CASE(scan_local_first_small_array_fail)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(5000);
    s32 values[] = {0, 2, 50, 1, 700};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first2(scratch.arena, values, array_capacity(values), cmp);

    REQUIRE(results.count == 0);
}

TEST_CASE(scan_local_first_unaligned_values_with_unaligned_search)
{
    ScratchArena scratch = scratch_begin();

    s32 value = 123;

    usize memory_size = 1024;
    char *memory = (char *)arena_alloc_bytes(scratch.arena, memory_size, 1, alignof(s32));
    memset(memory, 0xCD, memory_size);

    // Copy value to unaligned addresses
    usize unaligned_offsets[] = {1, 7, 19, 987};
    for (usize i = 0; i < array_capacity(unaligned_offsets); ++i) {
        memcpy(memory + unaligned_offsets[i], &value, sizeof(value));

    }

    // Copy value to aligned addresses
    usize aligned_offsets[] = {12, 24, 788, 1020};
    for (usize i = 0; i < array_capacity(aligned_offsets); ++i) {
        memcpy(memory + aligned_offsets[i], &value, sizeof(value));

    }

    scan_TypedValue scan_val = scan_val_make_int32((u32)value);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_memory_first2(scratch.arena, memory, memory_size, cmp, 1);

    // Since we're searching unaligned addresses, we should find all the values we copied into the array.
    REQUIRE(results.count == 8);

    // The matches will be sorted by offset in ascending order.
    REQUIRE(test_check_local_match(results.data[0], scan_val, 1));
    REQUIRE(test_check_local_match(results.data[1], scan_val, 7));
    REQUIRE(test_check_local_match(results.data[2], scan_val, 12));
    REQUIRE(test_check_local_match(results.data[3], scan_val, 19));
    REQUIRE(test_check_local_match(results.data[4], scan_val, 24));
    REQUIRE(test_check_local_match(results.data[5], scan_val, 788));
    REQUIRE(test_check_local_match(results.data[6], scan_val, 987));
    REQUIRE(test_check_local_match(results.data[7], scan_val, 1020));
}

TEST_CASE(scan_local_first_unaligned_values_with_aligned_search)
{
    ScratchArena scratch = scratch_begin();

    s32 value = 123;

    usize memory_size = 1024;
    char *memory = (char *)arena_alloc_bytes(scratch.arena, memory_size, 1, alignof(s32));
    memset(memory, 0xCD, memory_size);

    // Copy value to unaligned addresses
    usize unaligned_offsets[] = {1, 7, 19, 987};
    for (usize i = 0; i < array_capacity(unaligned_offsets); ++i) {
        memcpy(memory + unaligned_offsets[i], &value, sizeof(value));

    }

    // Copy value to aligned addresses
    usize aligned_offsets[] = {12, 24, 788, 1020};
    for (usize i = 0; i < array_capacity(aligned_offsets); ++i) {
        memcpy(memory + aligned_offsets[i], &value, sizeof(value));

    }

    scan_TypedValue scan_val = scan_val_make_int32((u32)value);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    scan_ResultView results = test_scan_local_memory_first2(
        scratch.arena, memory, memory_size, cmp, alignof(s32));

    // Since we're searching aligned addresses, we should only find the values on aligned addresses.
    REQUIRE(results.count == 4);

    for (usize i = 0; i < results.count; ++i) {
        usize expected_offset = aligned_offsets[i];
        REQUIRE(test_check_local_match(results.data[i], scan_val, expected_offset));
    }
}

TEST_CASE(scan_local_first_repeating_unaligned_values_with_aligned_search)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(0xF00BF00B); // Scan value consists of two repeating 2-byte values
    s32 int_scan_val = (s32)scan_val.value.int32;
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    s32 buffer[] = {int_scan_val, int_scan_val, int_scan_val, 0};

    scan_ResultView matches = test_scan_local_memory_first2(scratch.arena, buffer, sizeof(buffer), cmp, 4);

    // If we only scan every 4th address from the base, it should find all the values.
    REQUIRE(matches.count == 3);
    REQUIRE(test_check_local_match(matches.data[0], scan_val, 0 * sizeof(s32)));
    REQUIRE(test_check_local_match(matches.data[1], scan_val, 1 * sizeof(s32)));
    REQUIRE(test_check_local_match(matches.data[2], scan_val, 2 * sizeof(s32)));
}

TEST_CASE(scan_local_first_repeating_unaligned_values_with_unaligned_search)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(0xF00BF00B); // Scan value consists of two repeating 2-byte values
    s32 int_scan_val = (s32)scan_val.value.int32;
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    s32 buffer[] = {int_scan_val, int_scan_val, int_scan_val, 0};

    scan_ResultView matches = test_scan_local_memory_first2(scratch.arena, buffer, sizeof(buffer), cmp, 4);

    // If we only scan every 2nd address from the base, every instance of the value should give two
    // matches except for the last one.
    matches = test_scan_local_memory_first2(scratch.arena, buffer, sizeof(buffer), cmp, 2);
    REQUIRE(matches.count == 5);
    REQUIRE(test_check_local_match(matches.data[0], scan_val, 0 * sizeof(s32) + 0));
    REQUIRE(test_check_local_match(matches.data[1], scan_val, 0 * sizeof(s32) + 2));
    REQUIRE(test_check_local_match(matches.data[2], scan_val, 1 * sizeof(s32) + 0));
    REQUIRE(test_check_local_match(matches.data[3], scan_val, 1 * sizeof(s32) + 2));
    REQUIRE(test_check_local_match(matches.data[4], scan_val, 2 * sizeof(s32) + 0));
}

// ****************************************************************************************************


/*
  TODO:
  - Use scratch arenas in tests instead of manually creating and destroying
  - Flatten match list when checking results
 */

#define TEST_SCAN_ARENA_SIZE MB(8)

#define test_scan_local_array_first(arena, data, count, cmp)            \
    scan_local_memory_first((arena), (data), sizeof(*(data)) * (count), (uintptr_t)(data), (cmp), ALIGNOF_VALUE(*(data)))

#define test_scan_local_array_next(arena, data, count, cmp, prev_matches) \
    scan_local_memory_next((arena), (data), sizeof(*(data)) * (count), (cmp), (prev_matches))

#define test_scan_local_memory_first(arena, data, size, cmp, align)     \
    scan_local_memory_first((arena), (data), (size), (uintptr_t)(data), (cmp), (align))

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
