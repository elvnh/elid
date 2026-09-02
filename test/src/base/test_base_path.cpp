TEST_CASE(path_filename_basic)
{
    REQUIRE_STR_EQ(path_filename(str_lit("/home/foo.txt")), str_lit("foo.txt"));
    REQUIRE_STR_EQ(path_filename(str_lit("/home/foo")), str_lit("foo"));
    REQUIRE_STR_EQ(path_filename(str_lit("foo.txt")), str_lit("foo.txt"));
}

TEST_CASE(path_no_filename)
{
    String s = path_filename(str_lit("/"));
    REQUIRE(str_is_null_or_empty(s));

    s = path_filename(str_lit(""));
    REQUIRE(str_is_null_or_empty(s));
}
