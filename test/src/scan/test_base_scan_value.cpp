TEST_CASE(scan_value_equal_int32)
{
    scan_Value a = {};
    scan_Value b = {};

    a.int32 = 123;
    b.int32 = 123;

    REQUIRE(scan_value_equal(SCAN_TYPE_INT32, a, b));

    b.int32 = 456;

    REQUIRE(!scan_value_equal(SCAN_TYPE_INT32, a, b));
}

TEST_CASE(scan_value_greater_than_int32)
{
    scan_Value a = {};
    scan_Value b = {};

    a.int32 = 123;
    b.int32 = 5;

    REQUIRE(scan_value_greater_than(SCAN_TYPE_INT32, a, b));

    b.int32 = 456;

    REQUIRE(!scan_value_greater_than(SCAN_TYPE_INT32, a, b));
}

TEST_CASE(scan_value_equal_float32)
{
    scan_Value a = {};
    scan_Value b = {};

    a.float32 = 2.5f;
    b.float32 = 2.5f;

    REQUIRE(scan_value_equal(SCAN_TYPE_FLOAT32, a, b));

    b.float32 = 3.0f;

    REQUIRE(!scan_value_equal(SCAN_TYPE_FLOAT32, a, b));
}

TEST_CASE(scan_value_greater_than_float32)
{
    scan_Value a = {};
    scan_Value b = {};

    a.float32 = 2.5f;
    b.float32 = 1.5f;

    REQUIRE(scan_value_greater_than(SCAN_TYPE_FLOAT32, a, b));

    b.float32 = 3.0f;

    REQUIRE(!scan_value_greater_than(SCAN_TYPE_FLOAT32, a, b));
}

TEST_CASE(scan_value_add_int32)
{
    scan_Value a = {};
    scan_Value b = {};

    a.int32 = 7;
    b.int32 = 8;

    scan_Value c = scan_value_add(SCAN_TYPE_INT32, a, b);

    scan_Value expected = {};
    expected.int32 = 15;

    REQUIRE(scan_value_equal(SCAN_TYPE_INT32, c, expected));
}

TEST_CASE(scan_value_add_float32)
{
    scan_Value a = {};
    scan_Value b = {};

    a.float32 = 3.2f;
    b.float32 = 2.1f;

    scan_Value c = scan_value_add(SCAN_TYPE_FLOAT32, a, b);

    scan_Value expected = {};
    expected.float32 = 5.3f;

    REQUIRE(scan_value_equal(SCAN_TYPE_FLOAT32, c, expected));
}
