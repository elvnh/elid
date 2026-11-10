#include "base_string.h"

String str_from_cstr(const char *str)
{
    String result = {};

    usize length = strlen(str);

    result.data = (u8 *)str;
    result.count = length;

    return result;
}

char *str_to_cstr(Arena *arena, String str)
{
    char *result = 0;

    String null_terminated = str_null_terminate(arena, str);

    result = (char *)null_terminated.data;

    return result;
}

String str_alloc(Arena *arena, usize count)
{
    String result = {};

    result.data = arena_alloc_array(arena, u8, count);

    if (result.data) {
        result.count = count;
    }

    return result;
}

String str_alloc_cstr(Arena *arena, const char *cstr)
{
    String result = str_clone(arena, str_from_cstr(cstr));

    return result;
}

String str_clone(Arena *arena, String str)
{
    String result = {};

    if (str.count > 0) {
        result = str_alloc(arena, str.count);
        memcpy(result.data, str.data, str.count);
    }

    return result;
}

char *str_clone_cstr(Arena *arena, const char *str)
{
    usize length = strlen(str);

    char *result = arena_alloc_array(arena, char, length + 1);
    memcpy(result, str, length + 1);

    return result;
}

String str_concat(Arena *arena, String a, String b)
{
    String result = {};

    if (a.count == 0) {
        result = b;
    } else if (b.count == 0) {
        result = a;
    } else {
        usize total_size = a.count + b.count;
        result.data = arena_alloc_array(arena, u8, total_size);

        // TODO: check if we can get a contiguous allocation so we don't need to realloc
        if (result.data) {
            result.count = total_size;

            memcpy(result.data, a.data, a.count);
            memcpy(result.data + a.count, b.data, b.count);
        }
    }

    return result;
}

String str_null_terminate(Arena *arena, String str)
{
    // TODO: check if we can get a contiguous allocation so we don't need to realloc
    String result = {};

    if (str.count > 0) {
        result = str_alloc(arena, str.count + 1);
        result.count = str.count;

        memcpy(result.data, str.data, result.count);
        result.data[result.count] = '\0';
    }

    return result;
}

b32 str_is_null_or_empty(String str)
{
    b32 result = (str.data == 0) || (str.count == 0);

    if (!str.data) {
        ASSERT(str.count == 0);
    }

    return result;
}

b32 str_eq(String a, String b)
{
    b32 result = a.count == b.count;

    if (result) {
        for (usize i = 0; i < a.count; ++i) {
            if (str_at(a, i) != str_at(b, i)) {
                result = false;
                break;
            }
        }
    }

    return result;
}

b32 str_is_empty(String a)
{
    b32 result = a.count == 0;

    return result;
}

String str_slice(String str, usize index, usize count)
{
    String result = str;

    usize clamped_index = MIN(index, str.count);
    usize remainder = str.count - clamped_index;
    usize clamped_count = MIN(count, remainder);

    result.data += clamped_index;
    result.count = clamped_count;

    return result;
}

String str_slice_from(String str, usize index)
{
    String result = str_slice(str, index, str.count);

    return result;
}

String str_prefix(String str, usize count)
{
    String result = str_slice(str, 0, count);

    return result;
}

String str_postfix(String str, usize count)
{
    usize index = sat_sub_usize(str.count, count);
    String result = str_slice(str, index, count);

    return result;
}

String str_pop_front(String str, usize count)
{
    String result = {};

    result = str_slice_from(str, count);

    return result;
}

String str_pop_back(String str, usize count)
{
    usize remainder = sat_sub_usize(str.count, count);

    String result = str_prefix(str, remainder);

    return result;
}

b32 str_starts_with(String str, String pattern)
{
    b32 result = false;

    String substr = str_prefix(str, pattern.count);
    result = str_eq(substr, pattern);

    return result;
}

b32 str_ends_with(String str, String pattern)
{
    b32 result = false;

    String substr = str_postfix(str, pattern.count);
    result = str_eq(substr, pattern);

    return result;
}

usize str_common_prefix_length(String a, String b)
{
    usize result = 0;

    usize min_length = MIN(a.count, b.count);

    while ((result < min_length) && (a.data[result] == b.data[result])) {
        ++result;
    }

    return result;
}

Cut str_cut(String str, u8 ch)
{
    Cut result = {};
    result.delimiter = ch;

    usize_Maybe next = str_find_first_occurrence(str, ch);

    if (next.ok) {
        result.head = str_prefix(str, next.value);
        result.tail = str_slice_from(str, next.value + 1);
        result.delimiter_present = true;
    } else {
        result.head = str;
    }

    return result;
}

Cut str_cut_next(Cut cut)
{
    Cut result = str_cut(cut.tail, cut.delimiter);

    return result;
}

u8 str_at(String a, usize index)
{
    ASSERT(index < a.count);

    u8 result = a.data[index];

    return result;
}

u8 *str_begin(String str)
{
    u8 *result = 0;

    if (str.count) {
        result = str.data;
    }

    return result;
}

u8 *str_end(String str)
{
    u8 *result = 0;

    if (str.count) {
        result = str.data + str.count;
    }

    return result;
}

usize_Maybe str_find_first_occurrence(String str, u8 ch)
{
    usize_Maybe result = {};

    for (usize i = 0; i < str.count; ++i) {
        if (str.data[i] == ch) {
            result = SOME(i);
            break;
        }
    }

    return result;
}

usize_Maybe str_find_last_occurrence(String str, u8 ch)
{
    usize_Maybe result = {};

    for (usize i = sat_sub_usize(str.count, 1); i < str.count; --i) {
        if (str.data[i] == ch) {
            result = SOME(i);
            break;
        }
    }

    return result;
}

usize_Maybe str_find_first_occurrence_from(String str, u8 ch, usize index)
{
    usize_Maybe result = {};

    String substr = str_slice_from(str, index);

    result = str_find_first_occurrence(substr, ch);
    result.value = index + result.value;

    return result;
}

String str_format(Arena *arena, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    String result = str_format_v(arena, fmt, args);
    va_end(args);

    return result;
}

char *str_format_cstr(Arena *arena, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    String str = str_format_v(arena, fmt, args);
    va_end(args);

    // NOTE: str_format_v already null terminates since it uses vsnprintf
    char *result = (char *)str.data;

    return result;
}

String str_format_v(Arena *arena, const char *fmt, va_list args)
{
    String result = {};

    va_list args_copy;
    va_copy(args_copy, args);

    // NOTE: vsnprintf writes a null terminator, so we need to make room for it.
    s32 bytes_required = vsnprintf(0, 0, fmt, args) + 1;

    if (bytes_required > 0) {
        String buffer = str_alloc(arena, s32_to_usize(bytes_required));

        s32 chars_written = vsnprintf((char *)buffer.data, (usize)bytes_required, fmt, args_copy);

        if (chars_written > 0) {
            buffer.count = s32_to_usize(chars_written);

            ASSERT(chars_written == (bytes_required - 1));

            result = buffer;
        }

    }

    va_end(args_copy);

    return result;
}

/* Chars */
u8 char_to_upper(u8 ch)
{
    // ASSERT(char_is_alpha(ch));

    u8 result = ch;

    if (char_is_lower(ch)) {
        result -= ('a' - 'A');
    }

    return result;
}

b32 char_is_upper(u8 ch)
{
    b32 result = (ch >= 'A') && (ch <= 'Z');

    return result;
}

b32 char_is_lower(u8 ch)
{
    b32 result = (ch >= 'a') && (ch <= 'z');

    return result;
}

b32 char_is_alpha(u8 ch)
{
    b32 result = char_is_upper(ch) || char_is_lower(ch);

    return result;
}

/* String searching */
b32 str_search__cmp(u8 a, u8 b, StringSearchFlags flags)
{
    if (flags & STR_SEARCH_CASE_INSENSITIVE) {
        a = char_to_upper(a);
        b = char_to_upper(b);
    }

    b32 result = a == b;

    return result;
}

StringSearch str_search(String str, String pattern, StringSearchFlags flags)
{
    ASSERT(str.data);
    ASSERT(pattern.data);

    StringSearch result = {};

    if (pattern.count <= str.count) {
        for (usize i = 0; i <= str.count - pattern.count; ++i) {
            b32 match = true;

            for (usize j = 0; (j < pattern.count) && match; ++j) {
                u8 char_a = str.data[i + j];
                u8 char_b = pattern.data[j];

                if (!str_search__cmp(char_a, char_b, flags)) {
                    match = false;
                }
            }

            if (match) {
                result.match = true;
                result.index = i;
                break;
            }
        }
    }

    return result;
}
