/* stn - String To Number - string to integer parsing library

 LICENSE
   ********************************************************************************
   MIT License
   Copyright (c) 2026 elvnh

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
   ********************************************************************************

 USAGE
   To use this library, please do the following:
     #define STN_IMPLEMENTATION
     #include "stn.h"

   in ONE of your source files. This will create the implementation in that translation
   unit. Other files in your project can then include this header without the define
   preceding the include, giving them access to the declarations without making the
   definitions visible.

   Example usage:
     const char str[] = "123";
     int32_t n = 0;

     if (stn_parse_s32(str, sizeof(str) - 1, &n, 10)) {
         printf("Successfully parsed number: %d\n", n);
     } else {
         fprintf(stderr, "Error!\n");
     }

   This library provides parsing functions for signed and unsigned integers from 8 to 64
   bits wide. The parsing can be done in any base from 2 and 36, inclusive. If the parsing
   succeeded, 1 is returned from the function, and if it failed, 0 is returned. The value
   of the integer that was parsed is returned via a pointer provided to the function.

   The functions of the form stn_parse_* can parse any integer representable in the type
   in question. The same goes for the functions of the form stn_parse_*_ranged, but they
   also accept a bound that the integer must be contained in. If either of these conditions
   are not fulfilled, the functions fail.

   This library does not use null-terminated strings. All functions accept the input
   string as a char pointer and a length. This means that you can parse integers in the
   middle of a string without null terminating it.
 */

#ifndef STN_STRING_TO_NUMBER_H
#define STN_STRING_TO_NUMBER_H

#include <stdint.h>
#include <stddef.h>

/*
  TODO:
  - STN_(UN)LIKELY
  - allow making functions static
 */

// Prevent function name mangling.
#ifdef __cplusplus
extern "C" {
#endif

int  stn_parse_s8(const char *str,    size_t length, int8_t   *out, uint32_t base);
int  stn_parse_u8(const char *str,    size_t length, uint8_t  *out, uint32_t base);
int  stn_parse_s16(const char *str,   size_t length, int16_t  *out, uint32_t base);
int  stn_parse_u16(const char *str,   size_t length, uint16_t *out, uint32_t base);
int  stn_parse_s32(const char *str,   size_t length, int32_t  *out, uint32_t base);
int  stn_parse_u32(const char *str,   size_t length, uint32_t *out, uint32_t base);
int  stn_parse_s64(const char *str,   size_t length, int64_t  *out, uint32_t base);
int  stn_parse_u64(const char *str,   size_t length, uint64_t *out, uint32_t base);
int  stn_parse_usize(const char *str, size_t length, size_t   *out, uint32_t base);

int  stn_parse_s8_ranged(const char  *str,   size_t length, int8_t   *out, uint32_t base, int8_t   min, int8_t   max);
int  stn_parse_u8_ranged(const char  *str,   size_t length, uint8_t  *out, uint32_t base, uint8_t  min, uint8_t  max);
int  stn_parse_s16_ranged(const char *str,   size_t length, int16_t  *out, uint32_t base, int16_t  min, int16_t  max);
int  stn_parse_u16_ranged(const char *str,   size_t length, uint16_t *out, uint32_t base, uint16_t min, uint16_t max);
int  stn_parse_s32_ranged(const char *str,   size_t length, int32_t  *out, uint32_t base, int32_t  min, int32_t  max);
int  stn_parse_u32_ranged(const char *str,   size_t length, uint32_t *out, uint32_t base, uint32_t min, uint32_t max);
int  stn_parse_s64_ranged(const char *str,   size_t length, int64_t  *out, uint32_t base, int64_t  min, int64_t  max);
int  stn_parse_u64_ranged(const char *str,   size_t length, uint64_t *out, uint32_t base, uint64_t min, uint64_t max);
int  stn_parse_usize_ranged(const char *str, size_t length, size_t   *out, uint32_t base, size_t   min, size_t   max);

int  stn_parse_digit(char c, uint32_t *out, uint32_t base);

#endif // STN_STRING_TO_NUMBER_H

#ifdef STN_IMPLEMENTATION

#include <limits.h>

#ifdef STN_ENABLE_ASSERTS
#    include <assert.h>
#    define STN_ASSERT(e) assert(e)
#else
#    define STN_ASSERT(e)
#endif

// NOTE: These macros are only guaranteed to work with positive numbers as
// we don't need them to work in the general case.
// TODO: investigate if the builtins are actually faster.
#if defined(__GNUC__) && defined(__has_builtin) &&                                            \
    __has_builtin(__builtin_add_overflow) && __has_builtin(__builtin_mul_overflow)
#    define STN_SAFE_ADD_U8(a, b, result_ptr) !__builtin_add_overflow((a), (b), (result_ptr))
#    define STN_SAFE_MUL_U8(a, b, result_ptr) !__builtin_mul_overflow((a), (b), (result_ptr))
#
#    define STN_SAFE_ADD_U16(a, b, result_ptr) !__builtin_add_overflow((a), (b), (result_ptr))
#    define STN_SAFE_MUL_U16(a, b, result_ptr) !__builtin_mul_overflow((a), (b), (result_ptr))
#
#    define STN_SAFE_ADD_U32(a, b, result_ptr) !__builtin_add_overflow((a), (b), (result_ptr))
#    define STN_SAFE_MUL_U32(a, b, result_ptr) !__builtin_mul_overflow((a), (b), (result_ptr))
#
#    define STN_SAFE_ADD_U64(a, b, result_ptr) !__builtin_add_overflow((a), (b), (result_ptr))
#    define STN_SAFE_MUL_U64(a, b, result_ptr) !__builtin_mul_overflow((a), (b), (result_ptr))
#else
#    define STN_DEFINE_SAFE_OP(a, b, res, max, op, rev_op)                                    \
     (((a) <= ((max) rev_op (b))) ? ((*(res) = (a) op (b)), 1) : 0)
#
#    define STN_SAFE_ADD_U8(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT8_MAX,   +, -)
#    define STN_SAFE_MUL_U8(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT8_MAX,   *, /)
#
#    define STN_SAFE_ADD_U16(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT16_MAX, +, -)
#    define STN_SAFE_MUL_U16(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT16_MAX, *, /)
#
#    define STN_SAFE_ADD_U32(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT32_MAX, +, -)
#    define STN_SAFE_MUL_U32(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT32_MAX, *, /)
#
#    define STN_SAFE_ADD_U64(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT64_MAX, +, -)
#    define STN_SAFE_MUL_U64(a, b, res) STN_DEFINE_SAFE_OP(a, b, res, UINT64_MAX, *, /)
#endif

#define STN_DEFINE_UNSIGNED_PARSE_FNS(bits)                                 \
    int stn_parse_u##bits##_ranged(const char *str, size_t length,          \
        uint##bits##_t *out, uint32_t base,                                 \
        uint##bits##_t min, uint##bits##_t max)                             \
    {                                                                       \
        STN_ASSERT(min <= max);                                             \
                                                                            \
        uint##bits##_t value = 0;                                           \
        int result = length > 0;                                            \
                                                                            \
        for (size_t i = 0; i < length; ++i) {                               \
            uint32_t digit = 0;                                             \
            int digit_ok = stn_parse_digit(str[i], &digit, base);           \
                                                                            \
            int multiply_ok = STN_SAFE_MUL_U##bits(                         \
                value, (uint##bits##_t)base, &value);                       \
            int add_ok = STN_SAFE_ADD_U##bits(                              \
                value, (uint##bits##_t)digit, &value);                      \
                                                                            \
            if (!(digit_ok && multiply_ok && add_ok)) {                     \
                result = 0;                                                 \
                break;                                                      \
            }                                                               \
        }                                                                   \
                                                                            \
        int overflow = (value < min) || (value > max);                      \
                                                                            \
        if (overflow) {                                                     \
            result = 0;                                                     \
        }                                                                   \
                                                                            \
        *out = value;                                                       \
        return result;                                                      \
    }                                                                       \
                                                                            \
    int stn_parse_u##bits(const char *str, size_t length,                   \
        uint##bits##_t *out, uint32_t base)                                 \
    {                                                                       \
        return stn_parse_u##bits##_ranged(                                  \
            str, length, out, base, 0, UINT##bits##_MAX);                   \
    }

#define STN_DEFINE_SIGNED_PARSE_FNS(bits)                                   \
    int stn_parse_s##bits##_ranged(const char *str, size_t length,          \
        int##bits##_t *out, uint32_t base,                                  \
        int##bits##_t min, int##bits##_t max)                               \
    {                                                                       \
        int##bits##_t sign = 1;                                             \
                                                                            \
        if ((length > 1) && (str[0] == '-')) {                              \
            sign = -1;                                                      \
                                                                            \
            ++str;                                                          \
            --length;                                                       \
        }                                                                   \
                                                                            \
        int##bits##_t value = 0;                                            \
        uint##bits##_t unsigned_value = 0;                                  \
                                                                            \
        int result = stn_parse_u##bits(str, length, &unsigned_value, base); \
                                                                            \
        if (result) {                                                       \
            uint##bits##_t int_max = (uint##bits##_t)INT##bits##_MAX;       \
            uint##bits##_t abs_int_min = int_max + 1;                       \
                                                                            \
            if (unsigned_value <= int_max) {                                \
                value = (int##bits##_t)unsigned_value * sign;               \
            } else if ((unsigned_value == abs_int_min) && (sign == -1)) {   \
                value = INT##bits##_MIN;                                    \
            } else {                                                        \
                result = 0;                                                 \
            }                                                               \
                                                                            \
            int overflow = (value < min) || (value > max);                  \
                                                                            \
            if (overflow) {                                                 \
                result = 0;                                                 \
            }                                                               \
        }                                                                   \
                                                                            \
        *out = value;                                                       \
        return result;                                                      \
    }                                                                       \
                                                                            \
    int stn_parse_s##bits(const char *str, size_t length,                   \
        int##bits##_t *out, uint32_t base)                                  \
    {                                                                       \
        return stn_parse_s##bits##_ranged(                                  \
            str, length, out, base, INT##bits##_MIN, INT##bits##_MAX);      \
    }

STN_DEFINE_UNSIGNED_PARSE_FNS(8)
STN_DEFINE_UNSIGNED_PARSE_FNS(16)
STN_DEFINE_UNSIGNED_PARSE_FNS(32)
STN_DEFINE_UNSIGNED_PARSE_FNS(64)

STN_DEFINE_SIGNED_PARSE_FNS(8)
STN_DEFINE_SIGNED_PARSE_FNS(16)
STN_DEFINE_SIGNED_PARSE_FNS(32)
STN_DEFINE_SIGNED_PARSE_FNS(64)

int stn_parse_digit(char c, uint32_t *out, uint32_t base)
{
    // TODO: simplify this function
    STN_ASSERT(base > 0);
    STN_ASSERT(base <= 36);

    int result = 0;
    uint32_t value = 0;

    if ((base > 0) && (base <= 36)) {
        if (base <= 10) {
            if ((c >= '0') && (c < ('0' + (char)base))) {
                value = (uint32_t)((char)c - '0');
                result = 1;
            }
        } else {
            if ((c >= 'a') && (c <= 'z')) {
                // Convert case to upper
                c -= 'a' - 'A';
            }

            if ((c >= '0') && (c <= '9')) {
                value = (uint32_t)((char)c - '0');
                result = 1;
            } else if ((c >= 'A') && (c < ('A' + (char)(base - 10)))) {
                value = (uint32_t)((char)10 + c - 'A');
                result = 1;
            }
        }
    }

    *out = value;

    return result;
}

int stn_parse_usize(const char *str, size_t length, size_t *out, uint32_t base)
{
    int result = 0;

    #if SIZE_MAX == UINT8_MAX
        result = stn_parse_u8(str, length, out, base);
    #elif SIZE_MAX == UINT16_MAX
        result = stn_parse_u16(str, length, out, base);
    #elif SIZE_MAX == UINT32_MAX
        result = stn_parse_u32(str, length, out, base);
    #elif SIZE_MAX == UINT64_MAX
        result = stn_parse_u64(str, length, out, base);
    #endif

    return result;
}

int stn_parse_usize_ranged(const char *str, size_t length, size_t *out, uint32_t base, size_t min, size_t max)
{
    int result = 0;

    #if SIZE_MAX == UINT8_MAX
        result = stn_parse_u8_ranged(str, length, out, base, min, max);
    #elif SIZE_MAX == UINT16_MAX
        result = stn_parse_u16_ranged(str, length, out, base, min, max);
    #elif SIZE_MAX == UINT32_MAX
        result = stn_parse_u32_ranged(str, length, out, base, min, max);
    #elif SIZE_MAX == UINT64_MAX
        result = stn_parse_u64_ranged(str, length, out, base, min, max);
    #endif

    return result;
}

#endif // STN_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#undef STN_ASSERT

#undef STN_DEFINE_SAFE_OP

#undef STN_SAFE_ADD_S8
#undef STN_SAFE_ADD_U8
#undef STN_SAFE_MUL_S8
#undef STN_SAFE_MUL_U8

#undef STN_SAFE_ADD_S16
#undef STN_SAFE_ADD_U16
#undef STN_SAFE_MUL_S16
#undef STN_SAFE_MUL_U16

#undef STN_SAFE_ADD_S32
#undef STN_SAFE_ADD_U32
#undef STN_SAFE_MUL_S32
#undef STN_SAFE_MUL_U32

#undef STN_SAFE_ADD_S64
#undef STN_SAFE_ADD_U64
#undef STN_SAFE_MUL_S64
#undef STN_SAFE_MUL_U64

#undef STN_DEFINE_SIGNED_PARSE_FNS
#undef STN_DEFINE_UNSIGNED_PARSE_FNS
