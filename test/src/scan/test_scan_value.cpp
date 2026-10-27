// TODO: when other types are supported, create tests for those as well

/*********/
/* INT32 */
/*********/

/* Equal */
TEST_CASE(scan_val_eq_int32_success_both_positive)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(scan_val_eq(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_eq_int32_fail_both_positive)
{
    scan_Value lhs = scan_val_make_int32(8).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(!scan_val_eq(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_eq_int32_success_both_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32((u32)-10).value;
    REQUIRE(scan_val_eq(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_eq_int32_fail_both_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32((u32)-11).value;
    REQUIRE(!scan_val_eq(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_eq_int32_fail_lhs_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(!scan_val_eq(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_eq_int32_fail_rhs_negative)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32((u32)-10).value;
    REQUIRE(!scan_val_eq(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

/* Greater than */
TEST_CASE(scan_val_gt_int32_success_both_positive)
{
    scan_Value lhs = scan_val_make_int32(12).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_fail_both_positive)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32(12).value;
    REQUIRE(!scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_success_both_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32((u32)-12).value;
    REQUIRE(scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_fail_both_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-12).value;
    scan_Value rhs = scan_val_make_int32((u32)-10).value;
    REQUIRE(!scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_fail_both_positive_and_equal)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(!scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_fail_both_negative_and_equal)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32((u32)-10).value;
    REQUIRE(!scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_false_positive_lhs_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32(15).value;
    REQUIRE(scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_false_positive_rhs_negative)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32((u32)-15).value;
    REQUIRE(scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_success_rhs_negative)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32((u32)-15).value;
    REQUIRE(scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

/* Less than */
TEST_CASE(scan_val_lt_int32_success_both_positive)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32(12).value;
    REQUIRE(scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_lt_int32_fail_both_positive)
{
    scan_Value lhs = scan_val_make_int32(12).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(!scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_lt_int32_success_both_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-12).value;
    scan_Value rhs = scan_val_make_int32((u32)-10).value;
    REQUIRE(scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_lt_int32_fail_both_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32((u32)-12).value;
    REQUIRE(!scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_lt_int32_fail_both_positive_and_equal)
{
    scan_Value lhs = scan_val_make_int32(10).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(!scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_lt_int32_fail_both_negative_and_equal)
{
    scan_Value lhs = scan_val_make_int32((u32)-10).value;
    scan_Value rhs = scan_val_make_int32((u32)-10).value;
    REQUIRE(!scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_lt_int32_false_positive)
{
    scan_Value lhs = scan_val_make_int32(1).value;
    scan_Value rhs = scan_val_make_int32(U32_MAX).value;
    REQUIRE(scan_val_lt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}

TEST_CASE(scan_val_gt_int32_success_lhs_negative)
{
    scan_Value lhs = scan_val_make_int32((u32)-15).value;
    scan_Value rhs = scan_val_make_int32(10).value;
    REQUIRE(scan_val_gt(SCAN_VALUE_TYPE_INT32, lhs, rhs));
}
