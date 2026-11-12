/* Utilities */
#define TEST_SCAN_LARGE_ARRAY_SIZE MB(64)

// NOTE: these macros provide 0 as the base address, so all addresses in the results will simply be
// an offset from the memory base.

// For searching aligned values in an array.
#define test_scan_local_array_first(arena, data, count, cmp)            \
    test_make_local_scan_results((arena),                               \
        scan_local_memory_first((arena), (data), sizeof(*(data)) * (count), 0, (cmp), ALIGNOF_VALUE(*(data))))

#define test_scan_local_array_next(arena, data, count, cmp, prev_matches) \
    test_make_local_scan_results((arena),                               \
        scan_local_memory_next((arena), (data), sizeof(*(data)) * (count), 0, (cmp), (prev_matches)))

// For searching values with arbitrary alignment in an array.
#define test_scan_local_memory_first(arena, data, size, cmp, align)     \
    test_make_local_scan_results((arena),                               \
        scan_local_memory_first((arena), (data), size, 0, (cmp), (align)))

#define test_scan_local_memory_next(arena, data, size, cmp, prev_matches) \
    test_make_local_scan_results((arena),                               \
        scan_local_memory_next((arena), (data), size, 0, (cmp), (prev_matches)))


struct test_LocalScanResults {
    scan_MatchList *matches;
    scan_ResultView view;
};

static test_LocalScanResults test_make_local_scan_results(Arena *arena, scan_MatchList *matches)
{
    test_LocalScanResults result = {};
    result.matches = matches;
    result.view = scan_result_view_from_match_list(arena, matches);

    return result;
}

static b32 test_check_local_match(scan_MatchView details, scan_TypedValue expected_value, uintptr_t expected_offset)
{
    // Since we created a scan_ResultView from a scan_MatchList, the region the matches are located
    // in are unknown, so retrieving the address will just return an offset.
    b32 result = (details.match->address == expected_offset)
        && scan_val_eq(expected_value.type, expected_value.value, details.match->value);

    return result;
}

static b32 test_result_views_eq(scan_ValueType type, scan_ResultView lhs, scan_ResultView rhs)
{
    b32 result = lhs.count == rhs.count;

    for (usize i = 0; result && i < lhs.count; ++i) {
        scan_MatchView lhs_details = lhs.data[i];
        scan_MatchView rhs_details = rhs.data[i];

        if (!scan_val_eq(type, lhs_details.match->value, rhs_details.match->value)
            || (lhs_details.match->address != rhs_details.match->address)) {
            result = false;
        }
    }

    return result;
}

static void test_scatter_value_(void *array, usize array_size, scan_TypedValue value, usize *indices, usize index_count)
{
    usize value_size = scan_val_type_get_size(value.type);

    for (usize i = 0; i < index_count; ++i) {
        usize index = indices[i];
        ASSERT(index + value_size <= array_size);

        memcpy((u8 *)array + index, &value.value, value_size);
    }
}
#define test_scatter_value(array, array_size, scan_value, indices) \
    test_scatter_value_((array), (array_size), (scan_value), (indices), array_capacity((indices)))

/* First scan */
TEST_CASE(scan_local_first_single_value_success)
{
    ScratchArena scratch = scratch_begin();

    s32 buffer = 123;

    scan_TypedValue scan_val = scan_val_make_int32((u32)buffer);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first(scratch.arena, &buffer, 1, cmp).view;

    REQUIRE(results.count == 1);
    REQUIRE(test_check_local_match(results.data[0], scan_val, 0));
}

TEST_CASE(scan_local_first_single_value_fail)
{
    ScratchArena scratch = scratch_begin();

    s32 buffer = 123;

    scan_TypedValue scan_val = scan_val_make_int32(5);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first(scratch.arena, &buffer, 1, cmp).view;

    REQUIRE(results.count == 0);
}

TEST_CASE(scan_local_first_small_array_success)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_array_first(scratch.arena, values, array_capacity(values), cmp).view;

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
    scan_ResultView results = test_scan_local_array_first(scratch.arena, values, array_capacity(values), cmp).view;

    REQUIRE(results.count == 0);
}

TEST_CASE(scan_local_first_unaligned_values_with_unaligned_search)
{
    ScratchArena scratch = scratch_begin();

    s32 value = 123;

    usize memory_size = 1024;
    char *memory = (char *)arena_alloc_bytes(scratch.arena, memory_size, 1, alignof(s32));
    memset(memory, 0xCD, memory_size);

    scan_TypedValue scan_val = scan_val_make_int32((u32)value);

    // Copy value to unaligned addresses
    usize unaligned_offsets[] = {1, 7, 19, 987};
    test_scatter_value(memory, memory_size, scan_val, unaligned_offsets);

    // Copy value to aligned addresses
    usize aligned_offsets[] = {12, 24, 788, 1020};
    test_scatter_value(memory, memory_size, scan_val, aligned_offsets);


    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_ResultView results = test_scan_local_memory_first(scratch.arena, memory, memory_size, cmp, 1).view;

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

    scan_ResultView results = test_scan_local_memory_first(
        scratch.arena, memory, memory_size, cmp, alignof(s32)).view;

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

    scan_ResultView matches = test_scan_local_memory_first(scratch.arena, buffer, sizeof(buffer), cmp, 4).view;

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

    scan_ResultView matches = test_scan_local_memory_first(scratch.arena, buffer, sizeof(buffer), cmp, 4).view;

    // If we only scan every 2nd address from the base, every instance of the value should give two
    // matches except for the last one.
    matches = test_scan_local_memory_first(scratch.arena, buffer, sizeof(buffer), cmp, 2).view;
    REQUIRE(matches.count == 5);
    REQUIRE(test_check_local_match(matches.data[0], scan_val, 0 * sizeof(s32) + 0));
    REQUIRE(test_check_local_match(matches.data[1], scan_val, 0 * sizeof(s32) + 2));
    REQUIRE(test_check_local_match(matches.data[2], scan_val, 1 * sizeof(s32) + 0));
    REQUIRE(test_check_local_match(matches.data[3], scan_val, 1 * sizeof(s32) + 2));
    REQUIRE(test_check_local_match(matches.data[4], scan_val, 2 * sizeof(s32) + 0));
}

TEST_CASE(scan_local_first_large_array_aligned)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(12345);
    usize alignment = alignof(s32);

    usize val_size = scan_val_type_get_size(scan_val.type);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    usize array_size = TEST_SCAN_LARGE_ARRAY_SIZE;
    u8 *array = arena_alloc_array(scratch.arena, u8, array_size);

    usize aligned_offsets[] = {0, 4, 32, 48, 1024, 880, array_size - val_size};

    for (usize i = 0; i < array_capacity(aligned_offsets); ++i) {
        ASSERT(is_aligned_pow2(aligned_offsets[i], alignment));

        memcpy(array + aligned_offsets[i], &scan_val.value, val_size);
    }

    scan_ResultView matches = test_scan_local_memory_first(
         scratch.arena, array, array_size, cmp, alignment).view;

    REQUIRE(matches.count == array_capacity(aligned_offsets));

    for (usize i = 0; i < matches.count; ++i) {
        REQUIRE(scan_val_eq(scan_val.type, matches.data[i].match->value, scan_val.value));
    }
}

TEST_CASE(scan_local_first_large_array_unaligned)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(12345);

    usize val_size = scan_val_type_get_size(scan_val.type);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    usize array_size = TEST_SCAN_LARGE_ARRAY_SIZE;
    u8 *array = arena_alloc_array(scratch.arena, u8, array_size);

    // Both aligned and unaligned indices, we should find both.
    usize unaligned_offsets[] = {0, 5, 17, 29, 55, 1023, 501, array_size - val_size};

    for (usize i = 0; i < array_capacity(unaligned_offsets); ++i) {
        memcpy(array + unaligned_offsets[i], &scan_val.value, val_size);
    }

    scan_ResultView matches = test_scan_local_memory_first(
         scratch.arena, array, array_size, cmp, 1).view;

    REQUIRE(matches.count == array_capacity(unaligned_offsets));

    for (usize i = 0; i < matches.count; ++i) {
        REQUIRE(scan_val_eq(scan_val.type, matches.data[i].match->value, scan_val.value));
    }
}

/* Next scan */
TEST_CASE(scan_local_next_single_value_unchanged)
{
    ScratchArena scratch = scratch_begin();

    s32 buffer = 123;

    scan_TypedValue scan_val = scan_val_make_int32((u32)buffer);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    test_LocalScanResults first_results = test_scan_local_array_first(scratch.arena, &buffer, 1, cmp);

    scan_ResultView next_results = test_scan_local_array_next(scratch.arena, &buffer, 1, cmp, first_results.matches).view;
    REQUIRE(next_results.count == 1);

    REQUIRE(test_result_views_eq(scan_val.type, first_results.view, next_results));
}

TEST_CASE(scan_local_next_single_value_changed)
{
    ScratchArena scratch = scratch_begin();

    s32 buffer = 123;

    scan_TypedValue scan_val = scan_val_make_int32((u32)buffer);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_MatchList *first_results = test_scan_local_array_first(scratch.arena, &buffer, 1, cmp).matches;

    // If we change the value inbetween scans, it should no longer be found.
    buffer = 5000;

    scan_ResultView next_results = test_scan_local_array_next(scratch.arena, &buffer, 1, cmp, first_results).view;
    REQUIRE(next_results.count == 0);
}

TEST_CASE(scan_local_next_small_array_unchanged)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    test_LocalScanResults first_results = test_scan_local_array_first(
        scratch.arena, values, array_capacity(values), cmp);

    scan_ResultView next_results = test_scan_local_array_next(
        scratch.arena, values, array_capacity(values), cmp, first_results.matches).view;

    REQUIRE(test_result_views_eq(scan_val.type, first_results.view, next_results));
}

TEST_CASE(scan_local_next_small_array_change_one_match)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_MatchList *first_matches = test_scan_local_array_first(scratch.arena, values, array_capacity(values), cmp).matches;

    // Change one of the values inbetween scans.
    values[4] = 75;

    scan_ResultView next_results = test_scan_local_array_next(
        scratch.arena, values, array_capacity(values), cmp, first_matches).view;

    REQUIRE(next_results.count == 1);
    REQUIRE(next_results.count != first_matches->total_match_count);

    REQUIRE(test_check_local_match(next_results.data[0], scan_val, 2 * sizeof(s32)));
}

TEST_CASE(scan_local_next_small_array_change_all_matches)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    scan_MatchList *first_matches = test_scan_local_array_first(scratch.arena, values, array_capacity(values), cmp).matches;

    // Change all of the matches inbetween scans.
    values[2] = 75;
    values[4] = 75;

    scan_ResultView next_results = test_scan_local_array_next(
        scratch.arena, values, array_capacity(values), cmp, first_matches).view;

    REQUIRE(next_results.count == 0);
}

TEST_CASE(scan_local_next_small_array_change_non_match)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    test_LocalScanResults first_results = test_scan_local_array_first(
        scratch.arena, values, array_capacity(values), cmp);

    // Changing a non-match inbetween scans shouldn't make a difference.
    values[0] = 75;

    scan_ResultView next_results = test_scan_local_array_next(
        scratch.arena, values, array_capacity(values), cmp, first_results.matches).view;

    REQUIRE(test_result_views_eq(scan_val.type, first_results.view, next_results));
}

TEST_CASE(scan_local_first_small_array_different_arrays)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(123);
    s32 values[] = {0, 2, (s32)scan_val.value.int32, 1, (s32)scan_val.value.int32};

    scan_Comparison cmp = scan_cmp_make_exact(scan_val);
    test_LocalScanResults first_results = test_scan_local_array_first(
        scratch.arena, values, array_capacity(values), cmp);

    s32 values2[array_capacity(values)] = {};
    memcpy(values2, values, sizeof(values));

    scan_ResultView next_results = test_scan_local_array_next(
        scratch.arena, values2, array_capacity(values2), cmp, first_results.matches).view;

    REQUIRE(test_result_views_eq(scan_val.type, first_results.view, next_results));
}

TEST_CASE(scan_local_next_large_array_aligned_unchanged)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(12345);
    usize alignment = alignof(s32);

    usize val_size = scan_val_type_get_size(scan_val.type);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    usize array_size = TEST_SCAN_LARGE_ARRAY_SIZE;
    u8 *array = arena_alloc_array(scratch.arena, u8, array_size);

    usize aligned_offsets[] = {0, 4, 32, 48, 880, 1024, array_size - val_size};
    test_scatter_value(array, array_size, scan_val, aligned_offsets);

    test_LocalScanResults first_matches = test_scan_local_memory_first(
         scratch.arena, array, array_size, cmp, alignment);

    scan_ResultView next_matches = test_scan_local_memory_next(
         scratch.arena, array, array_size, cmp, first_matches.matches).view;

    REQUIRE(test_result_views_eq(scan_val.type, first_matches.view, next_matches));
}

TEST_CASE(scan_local_next_large_array_aligned_changed)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(12345);
    usize alignment = alignof(s32);

    usize val_size = scan_val_type_get_size(scan_val.type);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    usize array_size = TEST_SCAN_LARGE_ARRAY_SIZE;
    u8 *array = arena_alloc_array(scratch.arena, u8, array_size);

    usize aligned_offsets[] = {0, 4, 32, 48, 880, 1024, array_size - val_size};
    test_scatter_value(array, array_size, scan_val, aligned_offsets);

    test_LocalScanResults first_matches = test_scan_local_memory_first(
         scratch.arena, array, array_size, cmp, alignment);

    // Change some values between the two scans.
    memset(array + aligned_offsets[0], 0xFF, val_size);
    memset(array + aligned_offsets[3], 0xFF, val_size);
    memset(array + aligned_offsets[6], 0xFF, val_size);

    scan_ResultView next_matches = test_scan_local_memory_next(
         scratch.arena, array, array_size, cmp, first_matches.matches).view;

    REQUIRE(next_matches.count == (first_matches.view.count - 3));

    REQUIRE(test_check_local_match(next_matches.data[0], scan_val, aligned_offsets[1]));
    REQUIRE(test_check_local_match(next_matches.data[1], scan_val, aligned_offsets[2]));
    REQUIRE(test_check_local_match(next_matches.data[2], scan_val, aligned_offsets[4]));
    REQUIRE(test_check_local_match(next_matches.data[3], scan_val, aligned_offsets[5]));
}

TEST_CASE(scan_local_next_large_array_unaligned_unchanged)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(12345);
    usize alignment = alignof(s32);

    usize val_size = scan_val_type_get_size(scan_val.type);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    usize array_size = TEST_SCAN_LARGE_ARRAY_SIZE;
    u8 *array = arena_alloc_array(scratch.arena, u8, array_size);

    usize unaligned_offsets[] = {0, 5, 17, 29, 55, 1023, 501, array_size - val_size};

    test_scatter_value(array, array_size, scan_val, unaligned_offsets);

    test_LocalScanResults first_matches = test_scan_local_memory_first(
         scratch.arena, array, array_size, cmp, alignment);

    scan_ResultView next_matches = test_scan_local_memory_next(
         scratch.arena, array, array_size, cmp, first_matches.matches).view;

    REQUIRE(test_result_views_eq(scan_val.type, first_matches.view, next_matches));
}

TEST_CASE(scan_local_next_large_array_unaligned_changed)
{
    ScratchArena scratch = scratch_begin();

    scan_TypedValue scan_val = scan_val_make_int32(12345);

    usize val_size = scan_val_type_get_size(scan_val.type);
    scan_Comparison cmp = scan_cmp_make_exact(scan_val);

    usize array_size = TEST_SCAN_LARGE_ARRAY_SIZE;
    u8 *array = arena_alloc_array(scratch.arena, u8, array_size);

    usize unaligned_offsets[] = {0, 5, 17, 29, 55, 1023, 501, array_size - val_size};

    test_scatter_value(array, array_size, scan_val, unaligned_offsets);

    test_LocalScanResults first_matches = test_scan_local_memory_first(
         scratch.arena, array, array_size, cmp, 1);

    // Change some values between the two scans.
    memset(array + unaligned_offsets[0], 0xFF, val_size);
    memset(array + unaligned_offsets[3], 0xFF, val_size);
    memset(array + unaligned_offsets[6], 0xFF, val_size);

    scan_ResultView next_matches = test_scan_local_memory_next(
         scratch.arena, array, array_size, cmp, first_matches.matches).view;

    REQUIRE(next_matches.count == (first_matches.view.count - 3));

    REQUIRE(test_check_local_match(next_matches.data[0], scan_val, unaligned_offsets[1]));
    REQUIRE(test_check_local_match(next_matches.data[1], scan_val, unaligned_offsets[2]));
    REQUIRE(test_check_local_match(next_matches.data[2], scan_val, unaligned_offsets[4]));
    REQUIRE(test_check_local_match(next_matches.data[3], scan_val, unaligned_offsets[5]));
}
