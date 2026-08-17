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

TEST_CASE(safe_sub_overflow_negative_a)
{
    s64 result = 0;

    REQUIRE(!safe_sub_s64(-2, S64_MAX, &result));
    REQUIRE(!safe_sub_s64(-3, S64_MAX, &result));
    REQUIRE(!safe_sub_s64(S64_MIN, S64_MAX, &result));
}

TEST_CASE(safe_sub_negative_b)
{
    s64 result = 0;

    REQUIRE(!safe_sub_s64(0, S64_MIN, &result));
    REQUIRE(!safe_sub_s64(1, S64_MIN, &result));
    REQUIRE(!safe_sub_s64(2, S64_MIN, &result));

}

TEST_CASE(safe_sub_negative_nums)
{
    s64 result = 0;

    REQUIRE(safe_sub_s64(-1, S64_MIN, &result) && result == S64_MAX);
    REQUIRE(safe_sub_s64(-2, S64_MIN, &result) && result == S64_MAX - 1);
}

TEST_CASE(safe_mul_basic)
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

    REQUIRE(safe_mul_s64(S64_MAX / 2, 2, &result) && result == S64_MAX - 1);
}

TEST_CASE(safe_mul_overflow_positive_nums)
{
    s64 result = 0;

    REQUIRE(!safe_mul_s64(S64_MAX, 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, 4, &result));
    REQUIRE(!safe_mul_s64(S64_MAX - 1, 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX / 2 + 1, 2, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, S64_MAX, &result));
    REQUIRE(!safe_mul_s64(S64_MAX, S64_MAX - 1, &result));
}

TEST_CASE(safe_mul_overflow_negative_a)
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

TEST_CASE(safe_mul_overflow_negative_b)
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

TEST_CASE(safe_mul_overflow_negative_nums)
{
    s64 result = 0;

    REQUIRE(!safe_mul_s64(-S64_MAX, -2, &result));
    REQUIRE(!safe_mul_s64(-S64_MAX, -4, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  -2, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  -1, &result));
    REQUIRE(!safe_mul_s64(S64_MIN,  S64_MIN, &result));
}
