#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#define REQUIRE(e) ASSERT_ALWAYS(e)
#define REQUIRE_STR_EQ(a, b) REQUIRE(str_eq(a, b))

#include "base/base_all.cpp"

Arena *test_arena;

#include "base/test_base_all.cpp"

// TODO: script that generates test runner

int main()
{
    test_arena = arena_create(MB(16));

    test_base();

    return 0;
}
