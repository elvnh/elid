// success both negative
// success lhs negative
// success rhs negative
// fail both negative
// fail lhs negative
// fail rhs negative

/*****************'************/
/* Non-comparative scan modes */
/******************************/

/* Exact */
TEST_CASE(scan_cmp_exact_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_exact(scan_val_make_int32(10));
    REQUIRE(scan_cmp_match(scan_val_make_int32(10).value, cmp));
}

TEST_CASE(scan_cmp_exact_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_exact(scan_val_make_int32(10));
    REQUIRE(!scan_cmp_match(scan_val_make_int32(5).value, cmp));
}

TEST_CASE(scan_cmp_exact_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_exact(scan_val_make_int32((u32)-5));
    REQUIRE(scan_cmp_match(scan_val_make_int32((u32)-5).value, cmp));
}

TEST_CASE(scan_cmp_exact_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_exact(scan_val_make_int32((u32)-5));
    REQUIRE(!scan_cmp_match(scan_val_make_int32((u32)-50).value, cmp));
}

/* Unknown */
TEST_CASE(scan_cmp_unknown_success)
{
    scan_Comparison cmp = scan_cmp_make_unknown(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match(scan_val_make_int32(10).value, cmp));
    REQUIRE(scan_cmp_match(scan_val_make_int32(0).value, cmp));
    REQUIRE(scan_cmp_match(scan_val_make_int32((u32)-15).value, cmp));
    REQUIRE(scan_cmp_match(scan_val_make_int32(1000).value, cmp));
}

/* Greater than */
TEST_CASE(scan_cmp_gt_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_greater_than(scan_val_make_int32(10));
    REQUIRE(scan_cmp_match(scan_val_make_int32(15).value, cmp));
}

TEST_CASE(scan_cmp_gt_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_greater_than(scan_val_make_int32(15));
    REQUIRE(!scan_cmp_match(scan_val_make_int32(10).value, cmp));
}

TEST_CASE(scan_cmp_gt_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_greater_than(scan_val_make_int32((u32)-5));
    REQUIRE(scan_cmp_match(scan_val_make_int32((u32)-3).value, cmp));
}

TEST_CASE(scan_cmp_gt_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_greater_than(scan_val_make_int32((u32)-3));
    REQUIRE(!scan_cmp_match(scan_val_make_int32((u32)-5).value, cmp));
}

TEST_CASE(scan_cmp_gt_success_lhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_greater_than(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match(scan_val_make_int32((u32)-5).value, cmp));
}

TEST_CASE(scan_cmp_gt_success_rhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_greater_than(scan_val_make_int32((u32)-5));
    REQUIRE(scan_cmp_match(scan_val_make_int32(5).value, cmp));
}

/* Less than */
TEST_CASE(scan_cmp_lt_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_less_than(scan_val_make_int32(15));
    REQUIRE(scan_cmp_match(scan_val_make_int32(10).value, cmp));
}

TEST_CASE(scan_cmp_lt_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_less_than(scan_val_make_int32(10));
    REQUIRE(!scan_cmp_match(scan_val_make_int32(15).value, cmp));
}

TEST_CASE(scan_cmp_lt_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_less_than(scan_val_make_int32((u32)-3));
    REQUIRE(scan_cmp_match(scan_val_make_int32((u32)-5).value, cmp));
}

TEST_CASE(scan_cmp_lt_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_less_than(scan_val_make_int32((u32)-5));
    REQUIRE(!scan_cmp_match(scan_val_make_int32((u32)-3).value, cmp));
}

TEST_CASE(scan_cmp_lt_success_lhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_less_than(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match(scan_val_make_int32((u32)-5).value, cmp));
}

TEST_CASE(scan_cmp_lt_success_rhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_less_than(scan_val_make_int32((u32)-5));
    REQUIRE(scan_cmp_match(scan_val_make_int32(5).value, cmp));
}

/**************************/
/* Comparative scan modes */
/**************************/

/* Changed */
TEST_CASE(scan_cmp_changed_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_changed(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(8).value));
}

TEST_CASE(scan_cmp_changed_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_changed(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_changed_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_changed(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32((u32)-8).value));
}

TEST_CASE(scan_cmp_changed_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_changed(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32((u32)-10).value));
}

TEST_CASE(scan_cmp_changed_success_lhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_changed(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_changed_success_rhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_changed(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32((u32)-5).value));
}

/* Changed by... */
TEST_CASE(scan_cmp_changed_by_success_both_positive_increased)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(15).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_changed_by_success_both_positive_decreased)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(15).value));
}

TEST_CASE(scan_cmp_changed_by_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(5));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(14).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_changed_by_success_new_value_negative_increased)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32((u32)-10).value));
}

TEST_CASE(scan_cmp_changed_by_fail_new_value_negative)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(5));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32((u32)-8).value));
}

TEST_CASE(scan_cmp_changed_by_success_new_value_negative_decreased)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_changed_by_success_new_value_negative)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32(10));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-6).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_changed_by_success_negative_change_increased)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32((u32)-10));
    // The minus sign should just be negated.
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(15).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_changed_by_success_negative_change_decreased)
{
    scan_Comparison cmp = scan_cmp_make_changed_by(scan_val_make_int32((u32)-10));
    // The minus sign should just be negated.
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32(15).value));
}

/* Increased */
TEST_CASE(scan_cmp_increased_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_increased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(8).value));
}

TEST_CASE(scan_cmp_increased_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_increased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(10).value));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(8).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_increased_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_increased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-8).value, cmp, scan_val_make_int32((u32)-10).value));
}

TEST_CASE(scan_cmp_increased_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_increased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32((u32)-8).value));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32((u32)-10).value));
}

TEST_CASE(scan_cmp_increased_success_rhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_increased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32((u32)-5).value));
}

/* Increased by... */
TEST_CASE(scan_cmp_increased_by_success_both_positive_increased)
{
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(15).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_increased_by_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32(5));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(14).value, cmp, scan_val_make_int32(10).value));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_increased_by_success_new_value_change_negative)
{
    // Should be the same as checking for a decrease.
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32((u32)-10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32(15).value));
}

TEST_CASE(scan_cmp_increased_by_success_new_value_negative)
{
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32((u32)-10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_increased_by_fail_new_value_negative)
{
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32((u32)-10));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(3).value));
}

TEST_CASE(scan_cmp_increased_by_success_new_value_negative_increase)
{
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32(10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32((u32)-15).value));
}

TEST_CASE(scan_cmp_increased_by_success_new_value_negative_decrease)
{
    scan_Comparison cmp = scan_cmp_make_increased_by(scan_val_make_int32((u32)-10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-15).value, cmp, scan_val_make_int32((u32)-5).value));
}

/* Decreased */
TEST_CASE(scan_cmp_decreased_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_decreased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32(10).value));
}

TEST_CASE(scan_cmp_decreased_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_decreased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(10).value));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(8).value));
}

TEST_CASE(scan_cmp_decreased_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_decreased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32((u32)-8).value));
}

TEST_CASE(scan_cmp_decreased_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_decreased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-8).value, cmp, scan_val_make_int32((u32)-10).value));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-10).value, cmp, scan_val_make_int32((u32)-10).value));
}

TEST_CASE(scan_cmp_decreased_success_rhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_decreased(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(10).value));
}

/* Decreased by... */
TEST_CASE(scan_cmp_decreased_by_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_decreased_by(scan_val_make_int32(5));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(15).value));
}

TEST_CASE(scan_cmp_decreased_by_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_decreased_by(scan_val_make_int32(5));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(14).value));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(10).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_decreased_by_success_new_value_change_negative)
{
    // Should be the same as checking for a increase.
    scan_Comparison cmp = scan_cmp_make_decreased_by(scan_val_make_int32((u32)-10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(15).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_decreased_by_success_new_value_negative)
{
    scan_Comparison cmp = scan_cmp_make_decreased_by(scan_val_make_int32(10));
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_decreased_by_fail_new_value_negative)
{
    scan_Comparison cmp = scan_cmp_make_decreased_by(scan_val_make_int32(10));
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(3).value));
}

/* Unchanged */
TEST_CASE(scan_cmp_unchanged_success_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_unchanged(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_unchanged_success_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_unchanged(SCAN_VALUE_TYPE_INT32);
    REQUIRE(scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32((u32)-5).value));
}

TEST_CASE(scan_cmp_unchanged_fail_both_positive)
{
    scan_Comparison cmp = scan_cmp_make_unchanged(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32(4).value));
}

TEST_CASE(scan_cmp_unchanged_fail_both_negative)
{
    scan_Comparison cmp = scan_cmp_make_unchanged(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32((u32)-4).value));
}

TEST_CASE(scan_cmp_unchanged_fail_lhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_unchanged(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32((u32)-5).value, cmp, scan_val_make_int32(5).value));
}

TEST_CASE(scan_cmp_unchanged_fail_rhs_negative)
{
    scan_Comparison cmp = scan_cmp_make_unchanged(SCAN_VALUE_TYPE_INT32);
    REQUIRE(!scan_cmp_match_with_previous(scan_val_make_int32(5).value, cmp, scan_val_make_int32((u32)-5).value));
}
