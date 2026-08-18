#pragma once

struct String {
    u8   *data;
    usize count;
};

struct Cut {
    String head;
    String tail;
    b32    delimiter_present;
    u8     delimiter;
};

DEFINE_LIST_TYPES(String);

#define     str_lit(s) String{(u8 *)s, ARRAY_COUNT(s) - 1}
String      str_from_cstr(const char *str);
String      str_alloc_cstr(Arena *arena, const char *cstr);
String      str_alloc(Arena *arena, usize count);
String      str_copy(Arena *arena, String str);
String      str_concat(Arena *arena, String a, String b);
String      str_null_terminate(Arena *arena, String str);
u8          str_at(String a, usize index);
u8         *str_begin(String str);
u8         *str_end(String str);
b32         str_eq(String a, String b);
b32         str_is_empty(String a);
b32         str_starts_with(String str, String pattern);
b32         str_ends_with(String str, String pattern);
String      str_slice(String str, usize index, usize count);
String      str_slice_from(String str, usize index);
String      str_prefix(String str, usize count);
String      str_postfix(String str, usize count);
String      str_pop_front(String str, usize count);
String      str_pop_back(String str, usize count);
usize       str_common_prefix_length(String a, String b);
Cut         str_cut(String str, u8 ch);
Cut         str_cut_next(Cut cut);
usize_Maybe str_find_first_occurrence(String str, u8 ch);
usize_Maybe str_find_last_occurrence(String str, u8 ch);
usize_Maybe str_find_first_occurrence_from(String str, u8 ch, usize index);
String      str_format(Arena *arena, const char *fmt, ...) FORMAT_ATTRIBUTE(2, 3);
String      str_format_v(Arena *arena, const char *fmt, va_list args);
