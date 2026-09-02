TEST_CASE(str_lit_basic)
{
    String s = str_lit("abc");
    REQUIRE(s.count == 3);
    REQUIRE(memcmp(s.data, "abc", 3) == 0);
}

TEST_CASE(str_lit_empty)
{
    String s = str_lit("");
    REQUIRE(s.count == 0);
}

TEST_CASE(str_eq)
{
    REQUIRE(str_eq(str_lit("abc"), str_lit("abc")));
    REQUIRE(str_eq(str_lit(""), str_lit("")));

    REQUIRE(!str_eq(str_lit("abc"), str_lit("abcd")));
    REQUIRE(!str_eq(str_lit("abcd"), str_lit("abc")));
    REQUIRE(!str_eq(str_lit(""), str_lit(" ")));
}

TEST_CASE(str_from_cstr_basic)
{
    String s = str_from_cstr("abc");
    REQUIRE(s.count == 3);

    REQUIRE_STR_EQ(s, str_lit("abc"));
}

TEST_CASE(str_from_cstr_empty)
{
    String s = str_from_cstr("");
    REQUIRE(s.count == 0);

    REQUIRE_STR_EQ(s, str_lit(""));
}

TEST_CASE(str_copy_from_literal)
{
    String a = str_lit("abc");
    String b = str_copy(test_arena, a);

    REQUIRE_STR_EQ(a, b);
    REQUIRE(a.data != b.data);
    REQUIRE(a.count == b.count);
}

TEST_CASE(str_copy_from_heap)
{
    const char *literal = "abc";
    char *str = (char *)malloc(strlen(literal) + 1);
    strcpy(str, literal);

    String a = str_from_cstr(str);

    String b = str_copy(test_arena, a);
    REQUIRE_STR_EQ(a, b);

    free(str);
}

TEST_CASE(str_copy_empty_string)
{
    usize prev_mem_usage = arena_get_memory_usage(test_arena);

    String a = str_copy(test_arena, str_lit(""));

    REQUIRE(a.data == 0);
    REQUIRE(arena_get_memory_usage(test_arena) == prev_mem_usage);
}


TEST_CASE(str_copy_from_arena)
{
    String a = str_copy(test_arena, str_lit("abc"));
    String b = str_copy(test_arena, a);

    REQUIRE(str_eq(a, b));
    REQUIRE(a.data != b.data);
}

TEST_CASE(str_concat_literals)
{
    String a = str_lit("abc");
    String b = str_lit("def");

    String c = str_concat(test_arena, a, b);
    REQUIRE_STR_EQ(c, str_lit("abcdef"));

    REQUIRE(c.count == (a.count + b.count));
    REQUIRE(c.data != a.data);
    REQUIRE(c.data != b.data);
}

TEST_CASE(str_concat_heap_strings)
{
    String a = str_alloc_cstr(test_arena, "abc");
    String b = str_alloc_cstr(test_arena, "def");

    String c = str_concat(test_arena, a, b);

    REQUIRE_STR_EQ(c, str_lit("abcdef"));
    REQUIRE(c.count == (a.count + b.count));
}

TEST_CASE(str_concat_left_empty)
{
    String a = str_lit("");
    String b = str_lit("def");

    String c = str_concat(test_arena, a, b);
    REQUIRE_STR_EQ(c, str_lit("def"));
    REQUIRE(c.count == (a.count + b.count));
}

TEST_CASE(str_concat_right_empty)
{
    String a = str_lit("abc");
    String b = str_lit("");

    String c = str_concat(test_arena, a, b);
    REQUIRE_STR_EQ(c, str_lit("abc"));
    REQUIRE(c.count == (a.count + b.count));
}

TEST_CASE(str_concat_both_empty)
{
    String a = str_lit("");
    String b = str_lit("");

    String c = str_concat(test_arena, a, b);
    REQUIRE_STR_EQ(c, str_lit(""));
    REQUIRE(c.count == 0);
}

TEST_CASE(str_null_terminate_literal)
{
    String a = str_lit("abc");
    String b = str_null_terminate(test_arena, a);

    REQUIRE(b.count == a.count);
    REQUIRE_STR_EQ(a, b);
}

TEST_CASE(str_null_terminate_heap_string)
{
    String a = str_alloc_cstr(test_arena, "abc");
    String b = str_null_terminate(test_arena, a);

    REQUIRE(b.count == a.count);
    REQUIRE_STR_EQ(a, b);
}

TEST_CASE(str_starts_with)
{
    REQUIRE(str_starts_with(str_lit("abc"), str_lit("a")));
    REQUIRE(str_starts_with(str_lit("abc"), str_lit("ab")));
    REQUIRE(str_starts_with(str_lit("abc"), str_lit("abc")));

    REQUIRE(!str_starts_with(str_lit("abc"), str_lit("abcd")));

    REQUIRE(str_starts_with(str_lit("abc"), str_lit("")));
    REQUIRE(str_starts_with(str_lit(""), str_lit("")));

    REQUIRE(!str_starts_with(str_lit(""), str_lit("a")));
    REQUIRE(!str_starts_with(str_lit(""), str_lit("\0")));
}

TEST_CASE(str_ends_with)
{
    REQUIRE(str_ends_with(str_lit("abc"), str_lit("c")));
    REQUIRE(str_ends_with(str_lit("abc"), str_lit("bc")));
    REQUIRE(str_ends_with(str_lit("abc"), str_lit("abc")));

    REQUIRE(!str_ends_with(str_lit("abc"), str_lit("aabc")));

    REQUIRE(str_ends_with(str_lit("abc"), str_lit("")));
    REQUIRE(str_ends_with(str_lit(""), str_lit("")));

    REQUIRE(!str_ends_with(str_lit(""), str_lit("a")));
    REQUIRE(!str_ends_with(str_lit(""), str_lit("\0")));
}

TEST_CASE(str_slice_basic)
{
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 0, 1), str_lit("a"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 1, 1), str_lit("b"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 2, 1), str_lit("c"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 3, 1), str_lit(""));

    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 0, 2), str_lit("ab"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 1, 2), str_lit("bc"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 2, 2), str_lit("c"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 3, 1), str_lit(""));

    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 0, 3), str_lit("abc"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 1, 3), str_lit("bc"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 2, 3), str_lit("c"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 3, 1), str_lit(""));

    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 0, 0), str_lit(""));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 10, 0), str_lit(""));

    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 0, 10), str_lit("abc"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 1, 10), str_lit("bc"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 2, 10), str_lit("c"));
    REQUIRE_STR_EQ(str_slice(str_lit("abc"), 3, 10), str_lit(""));
}

TEST_CASE(str_prefix)
{
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 0), str_lit(""));
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 1), str_lit("a"));
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 2), str_lit("ab"));
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 3), str_lit("abc"));
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 4), str_lit("abcd"));
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 5), str_lit("abcd"));
    REQUIRE_STR_EQ(str_prefix(str_lit("abcd"), 10), str_lit("abcd"));

    REQUIRE_STR_EQ(str_prefix(str_lit(""), 0), str_lit(""));
    REQUIRE_STR_EQ(str_prefix(str_lit(""), 1), str_lit(""));
    REQUIRE_STR_EQ(str_prefix(str_lit(""), 2), str_lit(""));
}

TEST_CASE(str_postfix)
{
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 0), str_lit(""));
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 1), str_lit("d"));
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 2), str_lit("cd"));
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 3), str_lit("bcd"));
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 4), str_lit("abcd"));
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 5), str_lit("abcd"));
    REQUIRE_STR_EQ(str_postfix(str_lit("abcd"), 10), str_lit("abcd"));

    REQUIRE_STR_EQ(str_postfix(str_lit(""), 0), str_lit(""));
    REQUIRE_STR_EQ(str_postfix(str_lit(""), 1), str_lit(""));
    REQUIRE_STR_EQ(str_postfix(str_lit(""), 2), str_lit(""));
}

TEST_CASE(str_pop_front)
{
    REQUIRE_STR_EQ(str_pop_front(str_lit("abc"), 0), str_lit("abc"));
    REQUIRE_STR_EQ(str_pop_front(str_lit("abc"), 1), str_lit("bc"));
    REQUIRE_STR_EQ(str_pop_front(str_lit("abc"), 2), str_lit("c"));
    REQUIRE_STR_EQ(str_pop_front(str_lit("abc"), 3), str_lit(""));

    REQUIRE_STR_EQ(str_pop_front(str_lit(""), 0), str_lit(""));
    REQUIRE_STR_EQ(str_pop_front(str_lit(""), 1), str_lit(""));
    REQUIRE_STR_EQ(str_pop_front(str_lit(""), 2), str_lit(""));
}

TEST_CASE(str_pop_back)
{
    REQUIRE_STR_EQ(str_pop_back(str_lit("abc"), 0), str_lit("abc"));
    REQUIRE_STR_EQ(str_pop_back(str_lit("abc"), 1), str_lit("ab"));
    REQUIRE_STR_EQ(str_pop_back(str_lit("abc"), 2), str_lit("a"));
    REQUIRE_STR_EQ(str_pop_back(str_lit("abc"), 3), str_lit(""));

    REQUIRE_STR_EQ(str_pop_back(str_lit(""), 0), str_lit(""));
    REQUIRE_STR_EQ(str_pop_back(str_lit(""), 1), str_lit(""));
    REQUIRE_STR_EQ(str_pop_back(str_lit(""), 2), str_lit(""));
}

TEST_CASE(str_common_prefix_length)
{
    REQUIRE(str_common_prefix_length(str_lit("abc"), str_lit("abc")) == 3);
    REQUIRE(str_common_prefix_length(str_lit("ab"), str_lit("abc")) == 2);
    REQUIRE(str_common_prefix_length(str_lit("a"), str_lit("abc")) == 1);
    REQUIRE(str_common_prefix_length(str_lit(""), str_lit("abc")) == 0);

    REQUIRE(str_common_prefix_length(str_lit("abc"), str_lit("abc")) == 3);
    REQUIRE(str_common_prefix_length(str_lit("abc"), str_lit("ab")) == 2);
    REQUIRE(str_common_prefix_length(str_lit("abc"), str_lit("a")) == 1);
    REQUIRE(str_common_prefix_length(str_lit("abc"), str_lit("")) == 0);

    REQUIRE(str_common_prefix_length(str_lit("abc"), str_lit("abcd")) == 3);
    REQUIRE(str_common_prefix_length(str_lit("aba"), str_lit("abcd")) == 2);

    REQUIRE(str_common_prefix_length(str_lit("abcd"), str_lit("abc")) == 3);
    REQUIRE(str_common_prefix_length(str_lit("abcd"), str_lit("aba")) == 2);

    REQUIRE(str_common_prefix_length(str_lit(""), str_lit("")) == 0);
}

TEST_CASE(str_cut)
{
    String s = str_lit("abc/def/ghi");
    Cut cut = str_cut(s, '/');

    REQUIRE(cut.delimiter_present);
    REQUIRE_STR_EQ(cut.head, str_lit("abc"));
    REQUIRE_STR_EQ(cut.tail, str_lit("def/ghi"));

    cut = str_cut(cut.tail, '/');

    REQUIRE(cut.delimiter_present);
    REQUIRE_STR_EQ(cut.head, str_lit("def"));
    REQUIRE_STR_EQ(cut.tail, str_lit("ghi"));

    cut = str_cut(cut.tail, '/');

    REQUIRE(!cut.delimiter_present);
    REQUIRE_STR_EQ(cut.head, str_lit("ghi"));
}

TEST_CASE(str_cut_next)
{
    String s = str_lit("abc/def/ghi");
    Cut cut = str_cut(s, '/');

    REQUIRE(cut.delimiter_present);
    REQUIRE_STR_EQ(cut.head, str_lit("abc"));
    REQUIRE_STR_EQ(cut.tail, str_lit("def/ghi"));

    cut = str_cut_next(cut);

    REQUIRE(cut.delimiter_present);
    REQUIRE_STR_EQ(cut.head, str_lit("def"));
    REQUIRE_STR_EQ(cut.tail, str_lit("ghi"));

    cut = str_cut_next(cut);

    REQUIRE(!cut.delimiter_present);
    REQUIRE_STR_EQ(cut.head, str_lit("ghi"));
}

#define REQUIRE_MAYBE_EQ_TO(m, val) REQUIRE(MAYBE_EQ_TO((m), (val)))
#define REQUIRE_EMPTY_MAYBE(m) REQUIRE(!((m).ok))

TEST_CASE(str_find_first_occurrence)
{
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence(str_lit("abc/def/"), '/'), 3);
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence(str_lit("/abc/def/"), '/'), 0);

    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence(str_lit("abc"), '/'));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence(str_lit(""), '\0'));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence(str_lit(""), '/'));
}

TEST_CASE(str_find_last_occurrence)
{
    REQUIRE_MAYBE_EQ_TO(str_find_last_occurrence(str_lit("abc/def/"), '/'), 7);
    REQUIRE_MAYBE_EQ_TO(str_find_last_occurrence(str_lit("/abc/def/"), '/'), 8);

    REQUIRE_EMPTY_MAYBE(str_find_last_occurrence(str_lit("abc"), '/'));
    REQUIRE_EMPTY_MAYBE(str_find_last_occurrence(str_lit(""), '\0'));
    REQUIRE_EMPTY_MAYBE(str_find_last_occurrence(str_lit(""), '/'));
}

TEST_CASE(str_find_first_occurrence_from)
{
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence_from(str_lit("abc/def/"), '/', 0), 3);
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence_from(str_lit("abc/def/"), '/', 1), 3);
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence_from(str_lit("abc/def/"), '/', 2), 3);
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence_from(str_lit("abc/def/"), '/', 3), 3);

    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence_from(str_lit("/abc/def/"), '/', 0), 0);
    REQUIRE_MAYBE_EQ_TO(str_find_first_occurrence_from(str_lit("/abc/def/"), '/', 1), 4);

    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit("abc"), '/', 0));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit("abc"), '/', 1));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit("abc"), '/', 2));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit("abc"), '/', 3));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit("abc"), '/', 5));

    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit(""), '\0', 0));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit(""), '\0', 1));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit(""), '\0', 2));

    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit(""), '/', 0));
    REQUIRE_EMPTY_MAYBE(str_find_first_occurrence_from(str_lit(""), '/', 1));
}

TEST_CASE(str_format)
{
    REQUIRE_STR_EQ(str_format(test_arena, "%d", 123), str_lit("123"));
    REQUIRE_STR_EQ(str_format(test_arena, "%s %d", "abc", 123), str_lit("abc 123"));

    String s = str_format(test_arena, "%d", 1234);
    REQUIRE(s.count == 4);
}

