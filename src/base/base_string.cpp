#include "base_string.h"

String str_from_cstr(const char *str)
{
    String result = {};

    usize length = strlen(str);

    result.data = (u8 *)str;
    result.count = length;

    return result;
}

String str_concat(Arena *arena, String a, String b)
{
    ASSERT(a.count);
    ASSERT(b.count);

    String result = {};

    usize total_size = a.count + b.count;
    result.data = arena_alloc_array(arena, u8, total_size);

    // TODO: check if we can get a contiguous allocation so we don't need to realloc
    if (result.data) {
        result.count = total_size;

        memcpy(result.data, a.data, a.count);
        memcpy(result.data + a.count, b.data, b.count);
    }

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

String str_copy(Arena *arena, String str)
{
    ASSERT(str.count);

    String result = str_alloc(arena, str.count);
    memcpy(result.data, str.data, str.count);

    return result;
}

String str_null_terminate(Arena *arena, String str)
{
    // TODO: check if we can get a contiguous allocation so we don't need to realloc
    String result = str_alloc(arena, str.count + 1);
    result.count = str.count;

    memcpy(result.data, str.data, result.count);
    result.data[result.count] = '\0';

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

    usize remainder = sat_sub_usize(str.count, count);
    result = str_slice(str, count, remainder);

#if 0
    String result = str;

    usize n = MIN(str.count, count);
    result.data += n;
    result.count -= n;
#endif

    return result;
}

String str_pop_back(String str, usize count)
{
    usize remainder = sat_sub_usize(str.count, count);

    String result = str_slice(str, remainder, count);

#if 0
    String result = str;

    usize n = MIN(str.count, count);
    result.count -= n;
#endif

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

usize str_common_prefix(String a, String b)
{
    usize result = 0;

    usize min_length = MIN(a.count, b.count);

    while ((result < min_length) && (a.data[result] == b.data[result])) {
        ++result;
    }

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

Maybe_usize str_find_first_occurence(String str, u8 ch)
{
    Maybe_usize result = {};

    for (usize i = 0; i < str.count; ++i) {
        if (str.data[i] == ch) {
            result = SOME(i);
            break;
        }
    }

    return result;
}

Maybe_usize str_find_last_occurence(String str, u8 ch)
{
    Maybe_usize result = {};

    for (usize i = sat_sub_usize(str.count, 1); i < str.count; --i) {
        if (str.data[i] == ch) {
            result = SOME(i);
            break;
        }
    }

    return result;
}

Maybe_usize str_find_first_occurence_from(String str, u8 ch, usize index)
{
    Maybe_usize result = {};

    String substr = str_slice_from(str, index);

    result = str_find_first_occurence(substr, ch);
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

String str_format_v(Arena *arena, const char *fmt, va_list args)
{
    String result = {};

    va_list args_copy;
    va_copy(args_copy, args);

    // NOTE: vsnprintf writes a null terminator, so we need to make room for it.
    s32 bytes_required = vsnprintf(0, 0, fmt, args) + 1;

    if (bytes_required > 0) {
        String buffer = str_alloc(arena, s32_to_usize(bytes_required));

        s32 bytes_written = vsnprintf((char *)buffer.data, (usize)bytes_required, fmt, args_copy);

        if (bytes_written > 0) {
            buffer.count = s32_to_usize(bytes_written);

            ASSERT(bytes_written == bytes_required);

            result = buffer;
        }

    }

    va_end(args_copy);

    return result;
}
