TEST_CASE(scan_first_int_exact_basic)
{
    s32 values[] = {123, 20, 35312, 123};

    scan_Params params = {};
    params.mode = SCAN_MODE_EXACT;
    params.type = SCAN_TYPE_INT32;
    params.args[0].int32 = 123;

    scan_Result result = scan_local_memory_first(test_arena, values, sizeof(values), params, (uintptr_t)values, SCAN_DEFAULT_STRIDE);
    REQUIRE(result.match_count == 2);

    REQUIRE(result.matches[0].value.int32 == 123);
    REQUIRE(result.matches[0].remote_address == (uintptr_t)&values[0]);

    REQUIRE(result.matches[1].value.int32 == 123);
    REQUIRE(result.matches[1].remote_address == (uintptr_t)&values[3]);
}

TEST_CASE(scan_first_int_exact_non_aligned)
{
    char values[] = {1, 0, 1, 0, 1, 0, 1, 0};
    u32 search = 0;
    copy_mem(&search, values, 1, sizeof(search));

    scan_Params params = {};
    params.mode = SCAN_MODE_EXACT;
    params.type = SCAN_TYPE_INT32;
    params.args[0].int32 = (u32)search;

    scan_Result result = scan_local_memory_first(test_arena, values, sizeof(values), params, (uintptr_t)values, SCAN_DEFAULT_STRIDE);

    REQUIRE(result.match_count == 3);

    REQUIRE(result.matches[0].value.int32 == search);
    REQUIRE(result.matches[0].remote_address == (uintptr_t)&values[0]);

    REQUIRE(result.matches[1].value.int32 == search);
    REQUIRE(result.matches[1].remote_address == (uintptr_t)&values[2]);

    REQUIRE(result.matches[2].value.int32 == search);
    REQUIRE(result.matches[2].remote_address == (uintptr_t)&values[4]);
}

TEST_CASE(scan_first_int_exact_aligned_4_bytes)
{
    char values[] = {1, 0, 1, 0, 1, 0, 1, 0};
    u32 search = 0;
    copy_mem(&search, values, 1, sizeof(search));

    scan_Params params = {};
    params.mode = SCAN_MODE_EXACT;
    params.type = SCAN_TYPE_INT32;
    params.args[0].int32 = (u32)search;

    scan_Result result = scan_local_memory_first(test_arena, values, sizeof(values), params, (uintptr_t)values, 4);

    REQUIRE(result.match_count == 2);

    REQUIRE(result.matches[0].value.int32 == search);
    REQUIRE(result.matches[0].remote_address == (uintptr_t)&values[0]);

    REQUIRE(result.matches[1].value.int32 == search);
    REQUIRE(result.matches[1].remote_address == (uintptr_t)&values[4]);
}

TEST_CASE(scan_first_int_exact_aligned_8_bytes)
{
    char values[] = {1, 0, 1, 0, 1, 0, 1, 0};
    u32 search = 0;
    copy_mem(&search, values, 1, sizeof(search));

    scan_Params params = {};
    params.mode = SCAN_MODE_EXACT;
    params.type = SCAN_TYPE_INT32;
    params.args[0].int32 = (u32)search;

    scan_Result result = scan_local_memory_first(test_arena, values, sizeof(values), params, (uintptr_t)values, 8);

    REQUIRE(result.match_count == 1);

    REQUIRE(result.matches[0].value.int32 == search);
    REQUIRE(result.matches[0].remote_address == (uintptr_t)&values[0]);
}
