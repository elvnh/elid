// TODO: Clean up these tests

TEST_CASE(scan_compare_values_exact_int32)
{
    scan_Value a = scan_value_make_int32(123);
    scan_Params params = scan_params_make_eq(SCAN_TYPE_INT32, a);

    REQUIRE(scan_compare_values(scan_value_make_int32(123), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-123), params));
}

TEST_CASE(scan_compare_values_greater_than_int32)
{
    scan_Value a = scan_value_make_int32(123);
    scan_Params params = scan_params_make_gt(SCAN_TYPE_INT32, a);

    REQUIRE(scan_compare_values(scan_value_make_int32(124), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(1000), params));

    REQUIRE(!scan_compare_values(scan_value_make_int32(123), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(1), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(0), params));
}

TEST_CASE(scan_compare_values_greater_than_int32_limits)
{
    scan_Value a = scan_value_make_int32(U32_MAX - 1);
    scan_Params params = scan_params_make_gt(SCAN_TYPE_INT32, a);

    REQUIRE(scan_compare_values(scan_value_make_int32(U32_MAX), params));

    REQUIRE(!scan_compare_values(scan_value_make_int32(U32_MAX - 1), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(U32_MAX - 2), params));
}

// Ordered comparisons with ints where one side is negative will give some false
// positives/negatives, since the arguments will always be casted to unsigned.
TEST_CASE(scan_compare_values_greater_than_int32_false_positives)
{
    scan_Params params = scan_params_make_gt(SCAN_TYPE_INT32, scan_value_make_int32(123));

    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-123), params));
}

TEST_CASE(scan_compare_values_greater_than_int32_both_negative)
{
    scan_Value a = scan_value_make_int32((u32)-123);
    scan_Params params = scan_params_make_gt(SCAN_TYPE_INT32, a);

    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-122), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-120), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));

    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-123), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-125), params));
}

TEST_CASE(scan_compare_values_less_than_int32)
{
    scan_Params params = scan_params_make_lt(SCAN_TYPE_INT32, scan_value_make_int32(10));
    REQUIRE(scan_compare_values(scan_value_make_int32(8), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(7), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(3), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-123), params));

    REQUIRE(!scan_compare_values(scan_value_make_int32(10), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(12), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(123), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)S32_MAX), params));

    params = scan_params_make_lt(SCAN_TYPE_INT32, scan_value_make_int32((u32)-2));
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-3), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-1), params));
}

TEST_CASE(scan_compare_values_changed_int32)
{
    scan_Value previous = scan_value_make_int32(123);
    scan_Params params = scan_params_make_changed(SCAN_TYPE_INT32, previous);
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(!scan_compare_values(previous, params));

    previous = scan_value_make_int32((u32)-123);
    params = scan_params_make_changed(SCAN_TYPE_INT32, previous);
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(!scan_compare_values(previous, params));

    // NOTE: Expected false negative
    previous = scan_value_make_int32((u32)-1);
    params = scan_params_make_changed(SCAN_TYPE_INT32, previous);
    REQUIRE(!scan_compare_values(scan_value_make_int32(U32_MAX), params));
}

TEST_CASE(scan_compare_values_changed_by_int32)
{
    scan_Value previous = scan_value_make_int32(123);
    scan_Value change = scan_value_make_int32(1);
    scan_Params params = scan_params_make_changed_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(124), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(121), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(125), params));

    previous = scan_value_make_int32(1);
    change = scan_value_make_int32(2);
    params = scan_params_make_changed_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(3), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-2), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-3), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(4), params));

    previous = scan_value_make_int32((u32)-10);
    change = scan_value_make_int32(2);
    params = scan_params_make_changed_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-8), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-12), params));

    // "Changed by" with negative number doesn't make much sense but should work anyways.
    previous = scan_value_make_int32((u32)-10);
    change = scan_value_make_int32((u32)-2);
    params = scan_params_make_changed_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-8), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-12), params));
}

TEST_CASE(scan_compare_values_increased_int32)
{
    scan_Value previous = scan_value_make_int32(123);
    scan_Params params = scan_params_make_increased(SCAN_TYPE_INT32, previous);
    REQUIRE(scan_compare_values(scan_value_make_int32(124), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(125), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(1000), params));

    REQUIRE(!scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(121), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(0), params));

    // NOTE: expected false positive
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));

    previous = scan_value_make_int32(U32_MAX);
    params = scan_params_make_increased(SCAN_TYPE_INT32, previous);
    REQUIRE(scan_compare_values(scan_value_make_int32(U32_MAX + 1), params));
}

TEST_CASE(scan_compare_values_increased_by_int32)
{
    scan_Value previous = scan_value_make_int32(123);
    scan_Value change = scan_value_make_int32(1);
    scan_Params params = scan_params_make_increased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(124), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(123), params));

    previous = scan_value_make_int32(7);
    change = scan_value_make_int32(0);
    params = scan_params_make_increased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(7), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(8), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(6), params));

    previous = scan_value_make_int32((u32)-2);
    change = scan_value_make_int32(2);
    params = scan_params_make_increased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(U32_MAX + 1), params));

    previous = scan_value_make_int32((u32)-2);
    change = scan_value_make_int32(1);
    params = scan_params_make_increased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(U32_MAX), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-2), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-4), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(0), params));
}

TEST_CASE(scan_compare_values_decreased_int32)
{
    scan_Value previous = scan_value_make_int32(123);
    scan_Params params = scan_params_make_decreased(SCAN_TYPE_INT32, previous);
    REQUIRE(!scan_compare_values(scan_value_make_int32(123), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));

    previous = scan_value_make_int32(U32_MAX);
    params = scan_params_make_decreased(SCAN_TYPE_INT32, previous);
    REQUIRE(scan_compare_values(scan_value_make_int32(U32_MAX - 1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(U32_MAX + 1), params));

}

TEST_CASE(scan_compare_values_decreased_by_int32)
{
    scan_Value previous = scan_value_make_int32(123);
    scan_Value change = scan_value_make_int32(1);
    scan_Params params = scan_params_make_decreased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(123), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(124), params));

    previous = scan_value_make_int32(123);
    change = scan_value_make_int32(0);
    params = scan_params_make_decreased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32(123), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(122), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(124), params));

    previous = scan_value_make_int32((u32)-2);
    change = scan_value_make_int32(2);
    params = scan_params_make_decreased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-4), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(0), params));

    previous = scan_value_make_int32((u32)-2);
    change = scan_value_make_int32(1);
    params = scan_params_make_decreased_by(SCAN_TYPE_INT32, previous, change);
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-3), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32((u32)-4), params));
    REQUIRE(!scan_compare_values(scan_value_make_int32(0), params));
}

TEST_CASE(scan_compare_values_unknown_int32)
{
    scan_Params params = scan_params_make_unknown(SCAN_TYPE_INT32);
    REQUIRE(scan_compare_values(scan_value_make_int32(0), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(2), params));
    REQUIRE(scan_compare_values(scan_value_make_int32(1000), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-123), params));
    REQUIRE(scan_compare_values(scan_value_make_int32((u32)-1000), params));
}
