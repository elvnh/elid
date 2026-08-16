#pragma once

/* Basic types */
typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef bool     b8;
typedef int32_t  b32;

typedef ptrdiff_t ssize;
typedef size_t    usize;

typedef float  f32;
typedef double f64;

#define S8_MIN  INT8_MIN
#define S16_MIN INT16_MIN
#define S32_MIN INT32_MIN
#define S64_MIN INT64_MIN

#define S8_MAX  INT8_MAX
#define S16_MAX INT16_MAX
#define S32_MAX INT32_MAX
#define S64_MAX INT64_MAX

#define U8_MAX  UINT8_MAX
#define U16_MAX UINT16_MAX
#define U32_MAX UINT32_MAX
#define U64_MAX UINT64_MAX

#define S_SIZE_MAX PTRDIFF_MAX
#define U_SIZE_MAX SIZE_T_MAX

#define MIN_VALUE_OF(t) (IS_SIGNED(t) ? MIN_VALUE_OF_SIGNED(t) : 0)
#define MIN_VALUE_OF_SIGNED(t) (-MAX_VALUE_OF_SIGNED(t) - 1)

#define MAX_VALUE_OF(t) (IS_SIGNED(t) ? MAX_VALUE_OF_SIGNED(t) : MAX_VALUE_OF_UNSIGNED(t))
#define MAX_VALUE_OF_UNSIGNED(t) ((t)-1)
#define MAX_VALUE_OF_SIGNED(t) ((((t)1 << (TYPE_BIT_WIDTH(t) - 2)) - 1) * 2 + 1)

#define IS_UNSIGNED(t) (((t)-1) > ((t)0))
#define IS_SIGNED(t) !IS_UNSIGNED(t)

#define TYPE_BIT_WIDTH(t) (sizeof(t) * 8)

#define DEFINE_MAYBE_TYPE(t) struct Maybe_##t { t value; b32 ok; }
#define SOME(val) {val, true}

DEFINE_MAYBE_TYPE(usize);

/* Pseudo-keywords */
#define global static
#define thread_global global thread_local

/* Attributes */
#if COMPILER_GCC
#    define FORMAT_ATTRIBUTE(fmt_arg_number, var_arg_number)    \
    __attribute__((format(printf, fmt_arg_number, var_arg_number)))
#else
#    error FORMAT_ATTRIBUTE not defined for this compiler
#endif

/* Debugging */
#if DEBUG_BUILD
#    define ASSERT(e) ASSERT_ALWAYS(e)

#    define ASSERT_FAIL(str)                           \
     fprintf(stderr, "\n*** ASSERTION FAILED ***\n"  \
         "Expression: '%s'\nFunction: %s\n%s:%d:\n", \
         str, __func__, __FILE__, __LINE__)

// In debug builds, this will assert that the first expression is true before discarding
// it and returning the second expression. In release builds, it will simply return the
// second expressioon.
#    define ASSERT_AND_THEN(check, e) (ASSERT(check), (e))

#    define ALWAYS(e) ((e)  ? 1 : ASSERT_FAIL("ALWAYS("#e")"))
#    define NEVER(e)  (!(e) ? 0 : ASSERT_FAIL("NEVER("#e")"))
#    define TODO(e) ASSERT_FAIL("TODO("e")")
#else
#    define ASSERT(e) ((void)(e))
#    define ASSERT_AND_THEN(check, e) (e)
#    define ALWAYS(e) (e)
#    define NEVER(e)  (e)
#    define TODO(e)
#endif

#define ASSERT_ALWAYS(e) (!(e) ? ASSERT_FAIL(#e) : 0)

#if COMPILER_GCC
#    define DEBUG_BREAK() __builtin_trap()
#elif COMPILER_MSVC
#    define DEBUG_BREAK() __debug_break()
#else
#    error DEBUG_BREAK() not defined for this compiler
#endif

#define eval_print_s64(e) printf(#e" = %" PRId64 "\n", e);
#define eval_print_u64(e) printf(#e" = %" PRIu64 "\n", e);

#define print_string(s) printf("%.*s\n", (int)(s).count, (s).data);

/* Preprocessor */
#define CONCAT(a, b) CONCAT_(a, b)
#define CONCAT_() a ## b

/* General utilities */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define ARRAY_COUNT(a) (sizeof((a)) / sizeof(*(a)))
#define KB(n) ((u64)(n) * 1024u)
#define MB(n) (KB(n) * 1024u)
#define GB(n) (MB(n) * 1024u)
#define zero_mem(ptr, count) memset(ptr, 0, count)
#define ptr_byte_offset(ptr, count) (TYPEOF(ptr))(((u8 *)(ptr) + (count)))

#if COMPILER_GCC
#    define TYPEOF(t) __typeof__(t)
#elif COMPILER_MSVC
#    define TYPEOF(t) __typeof__(t)
#else
#    error TYPEOF() not defined for this compiler
#endif

/* Safe casts */
u32   s32_to_u32(s32 from);
usize s32_to_usize(s32 from);
s32   u32_to_s32(u32 from);

/* Safe arithmetic */
b32 safe_add_s32(s32 a, s32 b, s32 *res);
b32 safe_sub_s32(s32 a, s32 b, s32 *res);
b32 safe_mul_s32(s32 a, s32 b, s32 *res);
b32 safe_div_s32(s32 a, s32 b, s32 *res);
b32 safe_neg_s32(s32 a, s32 *res);

b32 safe_add_u32(u32 a, u32 b, u32 *res);
b32 safe_sub_u32(u32 a, u32 b, u32 *res);
b32 safe_mul_u32(u32 a, u32 b, u32 *res);
b32 safe_div_u32(u32 a, u32 b, u32 *res);

b32 safe_add_s64(s64 a, s64 b, s64 *res);
b32 safe_sub_s64(s64 a, s64 b, s64 *res);
b32 safe_mul_s64(s64 a, s64 b, s64 *res);
b32 safe_div_s64(s64 a, s64 b, s64 *res);
b32 safe_neg_s64(s64 a, s64 *res);

b32 safe_add_u64(u64 a, u64 b, u64 *res);
b32 safe_sub_u64(u64 a, u64 b, u64 *res);
b32 safe_mul_u64(u64 a, u64 b, u64 *res);
b32 safe_div_u64(u64 a, u64 b, u64 *res);

b32 safe_add_ssize(ssize a, ssize b, ssize *res);
b32 safe_sub_ssize(ssize a, ssize b, ssize *res);
b32 safe_mul_ssize(ssize a, ssize b, ssize *res);
b32 safe_div_ssize(ssize a, ssize b, ssize *res);
b32 safe_neg_ssize(ssize a, ssize *res);

b32 safe_add_usize(usize a, usize b, usize *res);
b32 safe_sub_usize(usize a, usize b, usize *res);
b32 safe_mul_usize(usize a, usize b, usize *res);
b32 safe_div_usize(usize a, usize b, usize *res);

/* Saturating arithmetic */
s64 sat_add_s64(s64 a, s64 b);
s64 sat_sub_s64(s64 a, s64 b);
s64 sat_mul_s64(s64 a, s64 b);
s64 sat_div_s64(s64 a, s64 b);

ssize sat_add_ssize(ssize a, ssize b);
ssize sat_sub_ssize(ssize a, ssize b);
ssize sat_mul_ssize(ssize a, ssize b);
ssize sat_div_ssize(ssize a, ssize b);

u64 sat_add_u64(u64 a, u64 b);
u64 sat_sub_u64(u64 a, u64 b);
u64 sat_mul_u64(u64 a, u64 b);
u64 sat_div_u64(u64 a, u64 b);

usize sat_add_usize(usize a, usize b);
usize sat_sub_usize(usize a, usize b);
usize sat_mul_usize(usize a, usize b);
usize sat_div_usize(usize a, usize b);

/* Math */
b32 is_pow2(u64 val);
b32 is_aligned(u64 val, u64 align);
b32 is_aligned_pow2(u64 val, u64 align);
u64 align_pow2(u64 val, u64 align);

/* ASan */
#if POISON_MEMORY
#    include <sanitizer/asan_interface.h>
#    define POISON(addr, size)  ASAN_POISON_MEMORY_REGION((addr), (size))
#    define UNPOISON(addr, size) ASAN_UNPOISON_MEMORY_REGION((addr), (size))
#else
#    define POISON(addr, size)
#    define UNPOISON(addr, size)
#endif
