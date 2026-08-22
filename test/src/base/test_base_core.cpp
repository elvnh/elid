/* Wrapping casts */
TEST_CASE(wrapping_cast)
{
    REQUIRE(wrap_u32_to_s32(1) == 1);
    REQUIRE(wrap_u32_to_s32(U32_MAX) == -1);
    REQUIRE(wrap_u32_to_s32((u32)S32_MAX + 1) == S32_MIN);
    REQUIRE(wrap_u32_to_s32((u32)S32_MAX + 2) == S32_MIN + 1);
}

/* Safe arithmetic */
// NOTE: We'll only test these functions for one signed and one unsigned type,
// as they're generated via macros and are identical except for the type.

TEST_CASE(safe_add_signed_basic)
{
    s64 result = 0;

    REQUIRE(safe_add_s64(1, 1, &result) && result == 2);
    REQUIRE(safe_add_s64(2, 2, &result) && result == 4);

    REQUIRE(safe_add_s64(0, S64_MAX, &result) && result == S64_MAX);
    REQUIRE(safe_add_s64(S64_MAX, 0, &result) && result == S64_MAX);
    REQUIRE(safe_add_s64(S64_MAX - 1, 1, &result) && result == S64_MAX);
}

TEST_CASE(safe_add_signed_negative)
{
    s64 result = 0;

    REQUIRE(safe_add_s64(S64_MIN, S64_MAX, &result) && result == -1);
    REQUIRE(safe_add_s64(S64_MIN, 0, &result) && result == S64_MIN);
    REQUIRE(safe_add_s64(S64_MIN + 1, -1, &result) && result == S64_MIN);
}

TEST_CASE(safe_add_signed_overflow_positive_nums)
{
    s64 result = 0;

    REQUIRE(!safe_add_s64(S64_MAX, 1, &result));
    REQUIRE(!safe_add_s64(S64_MAX, 2, &result));
    REQUIRE(!safe_add_s64(S64_MAX, S64_MAX, &result));
    REQUIRE(!safe_add_s64(S64_MAX - 1, 2, &result));
    REQUIRE(!safe_add_s64(S64_MAX - 2, 3, &result));
}

TEST_CASE(safe_add_signed_overflow_negative_nums)
{
    s64 result = 0;

    REQUIRE(!safe_add_s64(-1, S64_MIN, &result));
    REQUIRE(!safe_add_s64(S64_MIN / 2, S64_MIN, &result));
    REQUIRE(!safe_add_s64(S64_MIN, -1, &result));
    REQUIRE(!safe_add_s64(S64_MIN, -2, &result));
    REQUIRE(!safe_add_s64(S64_MIN, -3, &result));
}

TEST_CASE(safe_sub_signed_basic)
{
    s64 result = 0;

    REQUIRE(safe_sub_s64(0, 0, &result) && result == 0);
    REQUIRE(safe_sub_s64(0, 1, &result) && result == -1);
    REQUIRE(safe_sub_s64(1, 1, &result) && result == 0);
    REQUIRE(safe_sub_s64(-1, 1, &result) && result == -2);
    REQUIRE(safe_sub_s64(-2, 2, &result) && result == -4);
    REQUIRE(safe_sub_s64(-4, -2, &result) && result == -2);
}

TEST_CASE(safe_sub_signed_overflow_negative_a)
{
    s64 result = 0;

    REQUIRE(!safe_sub_s64(-2, S64_MAX, &result));
    REQUIRE(!safe_sub_s64(-3, S64_MAX, &result));
    REQUIRE(!safe_sub_s64(S64_MIN, S64_MAX, &result));
}

TEST_CASE(safe_sub_signed_negative_b)
{
    s64 result = 0;

    REQUIRE(!safe_sub_s64(0, S64_MIN, &result));
    REQUIRE(!safe_sub_s64(1, S64_MIN, &result));
    REQUIRE(!safe_sub_s64(2, S64_MIN, &result));

}

TEST_CASE(safe_sub_signed_negative_nums)
{
    s64 result = 0;

    REQUIRE(safe_sub_s64(-1, S64_MIN, &result) && result == S64_MAX);
    REQUIRE(safe_sub_s64(-2, S64_MIN, &result) && result == S64_MAX - 1);
}

TEST_CASE(safe_mul_signed_basic)
{
    s64 result = 0;

    REQUIRE(safe_mul_s64(0, 0, &result) && result == 0);
    REQUIRE(safe_mul_s64(1, 1, &result) && result == 1);
    REQUIRE(safe_mul_s64(2, 2, &result) && result == 4);
    REQUIRE(safe_mul_s64(2, -2, &result) && result == -4);
    REQUIRE(safe_mul_s64(-2, 2, &result) && result == -4);
    REQUIRE(safe_mul_s64(-2, -2, &result) && result == 4);

    REQUIRE(safe_mul_s64(1, S64_MAX, &result) && result == S64_MAX);
    REQUIRE(safe_mul_s64(S64_MAX, 1, &result) && result == S64_MAX);
    REQUIRE(safe_mul_s64(-1, S64_MAX, &result) && result == -S64_MAX);
    REQUIRE(safe_mul_s64(-S64_MAX, 1, &result) && result == -S64_MAX);
    REQUIRE(safe_mul_s64(-S64_MAX, -1, &result) && result == S64_MAX);

    REQUIRE(safe_mul_s64(S64_MAX, 0, &result) && result == 0);
    REQUIRE(safe_mul_s64(0, S64_MAX, &result) && result == 0);
    REQUIRE(safe_mul_s64(S64_MIN, 0, &result) && result == 0);
    REQUIRE(safe_mul_s64(0, S64_MIN, &result) && result == 0);

    REQUIRE(safe_mul_s64(S64_MAX / 2, 2, &result) && result == S64_MAX - 1);
}

TEST_CASE(safe_mul_signed_overflow_positive_nums)
{
    s64 result = 0;

    REQUIRE(!safe_mul_s64(S64_MAX, 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, 4, &result));
    REQUIRE(!safe_mul_s64(S64_MAX - 1, 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX / 2 + 1, 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, S64_MAX, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, S64_MAX - 1, &result));
}

TEST_CASE(safe_mul_signed_overflow_negative_a)
{
    s64 result = 0;

    REQUIRE(!safe_mul_s64(S64_MIN,  2, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  4, &result));
    REQUIRE(!safe_mul_s64(S64_MIN + 10,  2, &result));
    REQUIRE(!safe_mul_s64(S64_MIN / 2 - 1,  2, &result));
    REQUIRE(!safe_mul_s64(S64_MIN, S64_MAX, &result));
    REQUIRE(!safe_mul_s64(S64_MIN / 2, S64_MAX, &result));
    REQUIRE(!safe_mul_s64(-2, S64_MAX, &result));
}

TEST_CASE(safe_mul_signed_overflow_negative_b)
{
    s64 result = 0;

    REQUIRE(!safe_mul_s64(2, S64_MIN, &result));
    REQUIRE(!safe_mul_s64(4, S64_MIN, &result));
    REQUIRE(!safe_mul_s64(2, S64_MIN + 10, &result));
    REQUIRE(!safe_mul_s64(2, S64_MIN / 2 - 1, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, S64_MIN, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, S64_MIN / 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, -2, &result));
}

TEST_CASE(safe_mul_signed_overflow_negative_nums)
{
    s64 result = 0;

    REQUIRE(!safe_mul_s64(-S64_MAX, -2, &result));
    REQUIRE(!safe_mul_s64(-S64_MAX, -4, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  -2, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  -1, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  S64_MIN, &result));
}

TEST_CASE(safe_div_signed_basic)
{
    s64 result = 0;

    REQUIRE(safe_div_s64(4, 2, &result) && result == 2);
    REQUIRE(safe_div_s64(-4, 2, &result) && result == -2);
    REQUIRE(safe_div_s64(4, -2, &result) && result == -2);
    REQUIRE(safe_div_s64(-4, -2, &result) && result == 2);
    REQUIRE(safe_div_s64(0, 2, &result) && result == 0);
    REQUIRE(safe_div_s64(4, 1, &result) && result == 4);
    REQUIRE(safe_div_s64(1, 4, &result) && result == 0);
    REQUIRE(safe_div_s64(0, 4, &result) && result == 0);

    REQUIRE(safe_div_s64(S64_MAX, 2, &result) && result == S64_MAX / 2);
    REQUIRE(safe_div_s64(S64_MIN, 2, &result) && result == S64_MIN / 2);
    REQUIRE(safe_div_s64(S64_MAX, 1, &result) && result == S64_MAX);
    REQUIRE(safe_div_s64(S64_MIN, 1, &result) && result == S64_MIN);
    REQUIRE(safe_div_s64(S64_MAX, -1, &result) && result == -S64_MAX);

    REQUIRE(safe_div_s64(-1, S64_MIN, &result) && result == 0);

    REQUIRE(!safe_div_s64(2, 0, &result));
}

TEST_CASE(safe_div_signed_overflow)
{
    s64 result = 0;

    REQUIRE(!safe_div_s64(S64_MIN, -1, &result));
}

TEST_CASE(safe_neg_basic)
{
    s64 result = 0;

    REQUIRE(safe_neg_s64(1, &result) && result == -1);
    REQUIRE(safe_neg_s64(-1, &result) && result == 1);
    REQUIRE(safe_neg_s64(10, &result) && result == -10);
    REQUIRE(safe_neg_s64(-10, &result) && result == 10);
    REQUIRE(safe_neg_s64(0, &result) && result == 0);

    REQUIRE(safe_neg_s64(S64_MAX, &result) && result == -S64_MAX);
    REQUIRE(safe_neg_s64(-S64_MAX, &result) && result == S64_MAX);
}

TEST_CASE(safe_neg_overflow)
{
    s64 result = 0;

    REQUIRE(!safe_neg_s64(S64_MIN, &result));
}


TEST_CASE(safe_abs_basic)
{
    s64 result = 0;

    REQUIRE(safe_abs_s64(1, &result) && result == 1);
    REQUIRE(safe_abs_s64(-1, &result) && result == 1);
    REQUIRE(safe_abs_s64(S64_MAX, &result) && result == S64_MAX);
    REQUIRE(safe_abs_s64(-S64_MAX, &result) && result == S64_MAX);
}

TEST_CASE(safe_abs_overflow)
{
    s64 result = 0;

    REQUIRE(!safe_abs_s64(S64_MIN, &result));
}

TEST_CASE(safe_add_unsigned_basic)
{
    u64 result = 0;

    REQUIRE(safe_add_u64(1, 1, &result) && result == 2);
    REQUIRE(safe_add_u64(0, 1, &result) && result == 1);
    REQUIRE(safe_add_u64(1, 0, &result) && result == 1);
    REQUIRE(safe_add_u64(1000, 1000, &result) && result == 2000);

    REQUIRE(safe_add_u64(0, U64_MAX, &result) && result == U64_MAX);
    REQUIRE(safe_add_u64(U64_MAX, 0, &result) && result == U64_MAX);
    REQUIRE(safe_add_u64(U64_MAX - 1, 1, &result) && result == U64_MAX);
    REQUIRE(safe_add_u64(U64_MAX - 2, 2, &result) && result == U64_MAX);
}

TEST_CASE(safe_add_unsigned_overflow)
{
    u64 result = 0;

    REQUIRE(!safe_add_u64(U64_MAX, 1, &result));
    REQUIRE(!safe_add_u64(U64_MAX - 1, 2, &result));
    REQUIRE(!safe_add_u64(U64_MAX - 2, 3, &result));
    REQUIRE(!safe_add_u64(1, U64_MAX, &result));
    REQUIRE(!safe_add_u64(U64_MAX, 1, &result));
    REQUIRE(!safe_add_u64(U64_MAX, U64_MAX, &result));
    REQUIRE(!safe_add_u64(U64_MAX / 2, U64_MAX, &result));
}

TEST_CASE(safe_sub_unsigned_basic)
{
    u64 result = 0;

    REQUIRE(safe_sub_u64(1, 1, &result) && result == 0);
    REQUIRE(safe_sub_u64(2, 1, &result) && result == 1);
    REQUIRE(safe_sub_u64(U64_MAX, 1, &result) && result == U64_MAX - 1);
    REQUIRE(safe_sub_u64(U64_MAX, 2, &result) && result == U64_MAX - 2);
    REQUIRE(safe_sub_u64(U64_MAX, 0, &result) && result == U64_MAX);
    REQUIRE(safe_sub_u64(0, 0, &result) && result == 0);

    REQUIRE(safe_sub_u64(U64_MAX, U64_MAX, &result) && result == 0);
}

TEST_CASE(safe_sub_unsigned_overflow)
{
    u64 result = 0;

    REQUIRE(!safe_sub_u64(0, 1, &result));
    REQUIRE(!safe_sub_u64(0, 2, &result));
    REQUIRE(!safe_sub_u64(1, 2, &result));
    REQUIRE(!safe_sub_u64(2, 4, &result));

    REQUIRE(!safe_sub_u64(U64_MAX - 1, U64_MAX, &result));
}

TEST_CASE(safe_mul_unsigned_basic)
{
    u64 result = 0;

    REQUIRE(safe_mul_u64(1, 0, &result) && result == 0);
    REQUIRE(safe_mul_u64(0, 1, &result) && result == 0);
    REQUIRE(safe_mul_u64(1, 1, &result) && result == 1);
    REQUIRE(safe_mul_u64(1, 2, &result) && result == 2);
    REQUIRE(safe_mul_u64(2, 2, &result) && result == 4);
    REQUIRE(safe_mul_u64(1000, 1000, &result) && result == 1000 * 1000);

    REQUIRE(safe_mul_u64(U64_MAX, 0, &result) && result == 0);
    REQUIRE(safe_mul_u64(U64_MAX, 1, &result) && result == U64_MAX);
    REQUIRE(safe_mul_u64(U64_MAX / 2, 2, &result) && result == (U64_MAX / 2) * 2);
}

TEST_CASE(safe_mul_unsigned_overflow)
{
    u64 result = 0;

    REQUIRE(!safe_mul_u64(U64_MAX, 2, &result));
    REQUIRE(!safe_mul_u64(U64_MAX, 4, &result));
    REQUIRE(!safe_mul_u64(U64_MAX / 2, 4, &result));
}

TEST_CASE(safe_div_unsigned_basic)
{
    u64 result = 0;

    REQUIRE(safe_div_u64(10, 2, &result) && result == 5);
    REQUIRE(safe_div_u64(2, 2, &result) && result == 1);
    REQUIRE(safe_div_u64(10, 1, &result) && result == 10);
    REQUIRE(safe_div_u64(16, 4, &result) && result == 4);

    REQUIRE(safe_div_u64(U64_MAX, 2, &result) && U64_MAX / 2);

    REQUIRE(safe_div_u64(0, 10, &result) && result == 0);
}

TEST_CASE(safe_div_unsigned_overflow)
{
    u64 result = 0;

    REQUIRE(!safe_div_u64(1, 0, &result));
    REQUIRE(!safe_div_u64(10, 0, &result));
    REQUIRE(!safe_div_u64(U64_MAX, 0, &result));
    REQUIRE(!safe_div_u64(0, 0, &result));
}

/* Saturating arithmetic */
// Signed types
TEST_CASE(sat_add_signed_basic)
{
    REQUIRE(sat_add_s64(1, 1) == 2);
    REQUIRE(sat_add_s64(2, 2) == 4);
    REQUIRE(sat_add_s64(4, -2) == 2);
    REQUIRE(sat_add_s64(4, -6) == -2);
    REQUIRE(sat_add_s64(-2, -2) == -4);

    REQUIRE(sat_add_s64(2, 0) == 2);
    REQUIRE(sat_add_s64(0, 2) == 2);
}

TEST_CASE(sat_add_signed_near_limits)
{
    REQUIRE(sat_add_s64(S64_MAX - 1, 1) == S64_MAX);
    REQUIRE(sat_add_s64(S64_MIN + 1, -1) == S64_MIN);
    REQUIRE(sat_add_s64(S64_MAX, 0) == S64_MAX);
    REQUIRE(sat_add_s64(S64_MIN, 0) == S64_MIN);
    REQUIRE(sat_add_s64(S64_MIN, S64_MAX) == -1);
    REQUIRE(sat_add_s64(S64_MAX, S64_MIN) == -1);
}

TEST_CASE(sat_add_signed_saturate)
{
    REQUIRE(sat_add_s64(S64_MAX, 1) == S64_MAX);
    REQUIRE(sat_add_s64(S64_MAX, 1000) == S64_MAX);
    REQUIRE(sat_add_s64(S64_MAX, S64_MAX) == S64_MAX);

    REQUIRE(sat_add_s64(S64_MIN, -1) == S64_MIN);
    REQUIRE(sat_add_s64(S64_MIN, -1000) == S64_MIN);
    REQUIRE(sat_add_s64(S64_MIN, S64_MIN) == S64_MIN);
}

TEST_CASE(sat_sub_signed_basic)
{
    REQUIRE(sat_sub_s64(10, 6) == 4);
    REQUIRE(sat_sub_s64(1, 1) == 0);
    REQUIRE(sat_sub_s64(0, 2) == -2);
    REQUIRE(sat_sub_s64(2, 4) == -2);

    REQUIRE(sat_sub_s64(-2, -1) == -1);
    REQUIRE(sat_sub_s64(-4, -2) == -2);

    REQUIRE(sat_sub_s64(4, -3) == 7);

    REQUIRE(sat_sub_s64(-4, 3) == -7);

    REQUIRE(sat_sub_s64(2, 0) == 2);
    REQUIRE(sat_sub_s64(0, 0) == 0);
    REQUIRE(sat_sub_s64(0, 2) == -2);
}

TEST_CASE(sat_sub_signed_near_limits)
{
    REQUIRE(sat_sub_s64(S64_MAX, 1) == S64_MAX - 1);
    REQUIRE(sat_sub_s64(S64_MAX, 0) == S64_MAX);
    REQUIRE(sat_sub_s64(S64_MAX - 1, -1) == S64_MAX);
    REQUIRE(sat_sub_s64(S64_MAX, 5) == S64_MAX - 5);

    REQUIRE(sat_sub_s64(S64_MIN, -S64_MAX) == -1);
    REQUIRE(sat_sub_s64(S64_MAX, S64_MAX) == 0);
    REQUIRE(sat_sub_s64(S64_MIN, S64_MIN) == 0);

    REQUIRE(sat_sub_s64(S64_MIN, 0) == S64_MIN);
    REQUIRE(sat_sub_s64(S64_MIN + 1, 1) == S64_MIN);
    REQUIRE(sat_sub_s64(S64_MIN, -1) == S64_MIN + 1);
}

TEST_CASE(sat_sub_signed_saturate)
{
    REQUIRE(sat_sub_s64(S64_MIN, 1) == S64_MIN);
    REQUIRE(sat_sub_s64(S64_MIN, 1000) == S64_MIN);
    REQUIRE(sat_sub_s64(S64_MIN + 25, 1000) == S64_MIN);

    REQUIRE(sat_sub_s64(S64_MIN, S64_MAX) == S64_MIN);
    REQUIRE(sat_sub_s64(S64_MIN + 5, S64_MAX) == S64_MIN);

    REQUIRE(sat_sub_s64(S64_MAX, -5) == S64_MAX);
    REQUIRE(sat_sub_s64(S64_MAX, -S64_MAX) == S64_MAX);
    REQUIRE(sat_sub_s64(S64_MAX, S64_MIN) == S64_MAX);
    REQUIRE(sat_sub_s64(S64_MAX, S64_MIN) == S64_MAX);
}

TEST_CASE(sat_mul_signed_basic)
{
    REQUIRE(sat_mul_s64(2, 2) == 4);
    REQUIRE(sat_mul_s64(2, 4) == 8);
    REQUIRE(sat_mul_s64(2, 4) == 8);
    REQUIRE(sat_mul_s64(10, 12) == 120);

    REQUIRE(sat_mul_s64(-5, -2) == 10);

    REQUIRE(sat_mul_s64(5, -2) == -10);

    REQUIRE(sat_mul_s64(-5, 2) == -10);

    REQUIRE(sat_mul_s64(5, 0) == 0);
    REQUIRE(sat_mul_s64(0, 5) == 0);
}

TEST_CASE(sat_mul_signed_near_limits)
{
    REQUIRE(sat_mul_s64(S64_MAX, 0) == 0);
    REQUIRE(sat_mul_s64(S64_MAX, 1) == S64_MAX);
    REQUIRE(sat_mul_s64(S64_MAX / 2, 2) == S64_MAX - 1);

    REQUIRE(sat_mul_s64(S64_MAX, -1) == S64_MIN + 1);

    REQUIRE(sat_mul_s64(S64_MIN / 2, 2) == S64_MIN);
    REQUIRE(sat_mul_s64(S64_MIN / 4, 4) == S64_MIN);
}

TEST_CASE(sat_mul_signed_saturate)
{
    REQUIRE(sat_mul_s64(S64_MAX, 2) == S64_MAX);
    REQUIRE(sat_mul_s64(S64_MAX, 1000) == S64_MAX);
    REQUIRE(sat_mul_s64(S64_MAX, S64_MAX) == S64_MAX);
    REQUIRE(sat_mul_s64(S64_MAX / 2, 4) == S64_MAX);
    REQUIRE(sat_mul_s64(S64_MIN, -1) == S64_MAX);
    REQUIRE(sat_mul_s64(S64_MIN, S64_MIN) == S64_MAX);

    REQUIRE(sat_mul_s64(S64_MIN, 2) == S64_MIN);
    REQUIRE(sat_mul_s64(S64_MIN, 1000) == S64_MIN);
    REQUIRE(sat_mul_s64(S64_MIN, S64_MAX) == S64_MIN);
    REQUIRE(sat_mul_s64(S64_MIN / 2, 4) == S64_MIN);
}

TEST_CASE(sat_div_signed_basic)
{
    REQUIRE(sat_div_s64(4, 2) == 2);
    REQUIRE(sat_div_s64(4, 1) == 4);
    REQUIRE(sat_div_s64(4, 4) == 1);
    REQUIRE(sat_div_s64(0, 4) == 0);
    REQUIRE(sat_div_s64(1, 4) == 0);

    REQUIRE(sat_div_s64(-2, -2) == 1);
    REQUIRE(sat_div_s64(-4, -2) == 2);

    REQUIRE(sat_div_s64(4, -2) == -2);

    REQUIRE(sat_div_s64(-4, 2) == -2);
}

TEST_CASE(sat_div_signed_near_limits)
{
    REQUIRE(sat_div_s64(S64_MAX, 2) == S64_MAX / 2);
    REQUIRE(sat_div_s64(S64_MIN, 2) == S64_MIN / 2);
}

TEST_CASE(sat_div_signed_saturate)
{

    REQUIRE(sat_div_s64(S64_MIN, -1) == S64_MAX);

    // TODO: should this be the behaviour?
    REQUIRE(sat_div_s64(4, 0) == 0);
}

// Unsigned types
TEST_CASE(sat_add_unsigned_basic)
{
    REQUIRE(sat_add_u64(2, 2) == 4);
    REQUIRE(sat_add_u64(2, 5) == 7);

    REQUIRE(sat_add_u64(2, 0) == 2);
    REQUIRE(sat_add_u64(0, 2) == 2);
    REQUIRE(sat_add_u64(0, 1) == 1);

    REQUIRE(sat_add_u64(0, 0) == 0);
}

TEST_CASE(sat_add_unsigned_near_limits)
{
    REQUIRE(sat_add_u64(U64_MAX, 0) == U64_MAX);
    REQUIRE(sat_add_u64(U64_MAX - 1, 1) == U64_MAX);
    REQUIRE(sat_add_u64(U64_MAX - 2, 2) == U64_MAX);
    REQUIRE(sat_add_u64(0, U64_MAX) == U64_MAX);
}

TEST_CASE(sat_add_unsigned_saturate)
{
    REQUIRE(sat_add_u64(U64_MAX, 1) == U64_MAX);
    REQUIRE(sat_add_u64(U64_MAX, 1000) == U64_MAX);
    REQUIRE(sat_add_u64(U64_MAX, U64_MAX) == U64_MAX);

    REQUIRE(sat_add_u64(1, U64_MAX) == U64_MAX);
    REQUIRE(sat_add_u64(1000, U64_MAX) == U64_MAX);
}

TEST_CASE(sat_sub_unsigned_basic)
{
    REQUIRE(sat_sub_u64(10, 2) == 8);
    REQUIRE(sat_sub_u64(5, 2) == 3);

    REQUIRE(sat_sub_u64(5, 0) == 5);
}

TEST_CASE(sat_sub_unsigned_near_limits)
{
    REQUIRE(sat_sub_u64(0, 0) == 0);
    REQUIRE(sat_sub_u64(10, 10) == 0);
    REQUIRE(sat_sub_u64(U64_MAX, U64_MAX) == 0);
}

TEST_CASE(sat_sub_unsigned_saturate)
{
    REQUIRE(sat_sub_u64(0, 1) == 0);
    REQUIRE(sat_sub_u64(0, 2) == 0);
    REQUIRE(sat_sub_u64(0, U64_MAX) == 0);
    REQUIRE(sat_sub_u64(10, U64_MAX) == 0);
    REQUIRE(sat_sub_u64(U64_MAX - 1, U64_MAX) == 0);
}

TEST_CASE(sat_mul_unsigned_basic)
{
    REQUIRE(sat_mul_u64(2, 2) == 4);
    REQUIRE(sat_mul_u64(2, 4) == 8);

    REQUIRE(sat_mul_u64(2, 0) == 0);
    REQUIRE(sat_mul_u64(0, 2) == 0);

    REQUIRE(sat_mul_u64(4, 1) == 4);
    REQUIRE(sat_mul_u64(1, 4) == 4);
    REQUIRE(sat_mul_u64(1, 1) == 1);
}

TEST_CASE(sat_mul_unsigned_near_limits)
{
    REQUIRE(sat_mul_u64(U64_MAX, 1) == U64_MAX);
    REQUIRE(sat_mul_u64(U64_MAX / 2, 2) == U64_MAX - 1);

    REQUIRE(sat_mul_u64(U64_MAX, 0) == 0);

    REQUIRE(sat_mul_u64(0, 0) == 0);
}

TEST_CASE(sat_mul_unsigned_saturate)
{
    REQUIRE(sat_mul_u64(U64_MAX, 2) == U64_MAX);
    REQUIRE(sat_mul_u64(U64_MAX, 4) == U64_MAX);
    REQUIRE(sat_mul_u64(U64_MAX, 1000) == U64_MAX);
    REQUIRE(sat_mul_u64(U64_MAX, U64_MAX) == U64_MAX);
}

TEST_CASE(sat_div_unsigned_basic)
{
    REQUIRE(sat_div_u64(10, 5) == 2);
    REQUIRE(sat_div_u64(4, 2) == 2);
    REQUIRE(sat_div_u64(2, 2) == 1);
    REQUIRE(sat_div_u64(0, 10) == 0);

    REQUIRE(sat_div_u64(4, 1) == 4);
    REQUIRE(sat_div_u64(1, 4) == 0);
}

TEST_CASE(sat_div_unsigned_saturate)
{

    REQUIRE(sat_div_u64(10, 0) == 0);
    REQUIRE(sat_div_u64(0, 0) == 0);
}

/* Math */
TEST_CASE(is_pow2)
{
    REQUIRE(is_pow2(1));
    REQUIRE(is_pow2(2));
    REQUIRE(is_pow2(4));
    REQUIRE(is_pow2(8));
    REQUIRE(is_pow2(16));
    REQUIRE(is_pow2(65536));

    REQUIRE(!is_pow2(0));
    REQUIRE(!is_pow2(3));
    REQUIRE(!is_pow2(7));
    REQUIRE(!is_pow2(10));
    REQUIRE(!is_pow2(12));
    REQUIRE(!is_pow2(1023));
    REQUIRE(!is_pow2(65535));
}

TEST_CASE(is_aligned)
{
    REQUIRE(is_aligned(0, 4));
    REQUIRE(is_aligned(0, 8));
    REQUIRE(is_aligned(0, 10));
    REQUIRE(is_aligned(0, 12));
    REQUIRE(is_aligned(0, 16));

    REQUIRE(is_aligned(1, 1));
    REQUIRE(is_aligned(2, 1));
    REQUIRE(is_aligned(5, 1));
    REQUIRE(is_aligned(8, 1));
    REQUIRE(is_aligned(1231, 1));

    REQUIRE(is_aligned(7, 7));
    REQUIRE(is_aligned(14, 7));
    REQUIRE(!is_aligned(15, 7));
    REQUIRE(!is_aligned(16, 7));
}

TEST_CASE(is_aligned_pow2)
{
    REQUIRE(is_aligned_pow2(0, 4));
    REQUIRE(is_aligned_pow2(0, 8));
    REQUIRE(is_aligned_pow2(0, 16));

    REQUIRE(is_aligned(1, 1));
    REQUIRE(is_aligned(2, 1));
    REQUIRE(is_aligned(5, 1));
    REQUIRE(is_aligned(8, 1));
    REQUIRE(is_aligned(1231, 1));

    REQUIRE(is_aligned(8, 8));
    REQUIRE(is_aligned(16, 8));
    REQUIRE(is_aligned(24, 8));
    REQUIRE(is_aligned(32, 8));
    REQUIRE(!is_aligned(7, 8));
    REQUIRE(!is_aligned(9, 8));
    REQUIRE(!is_aligned(33, 8));
}

/* Maybe */
TEST_CASE(maybe_basic)
{
    s32_Maybe m = {};
    REQUIRE(!MAYBE_EQ_TO(m, 123));
    REQUIRE(!MAYBE_EQ_TO(m, 0));

    REQUIRE(!m.ok);

    m = SOME(123);
    REQUIRE(m.ok);
    REQUIRE(m.value == 123);

    REQUIRE(!MAYBE_EQ_TO(m, 0));
    REQUIRE(MAYBE_EQ_TO(m, 123));
}
