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
