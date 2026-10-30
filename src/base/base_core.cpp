#include "base_core.h"

/* Safe casts */
#define DEFINE_SAFE_CAST_SIGNED_TO_UNSIGNED(f, t)   \
    t f##_to_##t(f from)                            \
    {                                               \
        ASSERT_ALWAYS(from >= 0);                   \
        ASSERT_ALWAYS((t)from <= MAX_VALUE_OF(t));  \
        t result = (t)from;                         \
        return result;                              \
    }

#define DEFINE_SAFE_CAST_UNSIGNED_TO_SIGNED(f, t)   \
    t f##_to_##t(f from)                            \
    {                                               \
        ASSERT_ALWAYS(from <= MAX_VALUE_OF(t));     \
        t result = (t)from;                         \
        return result;                              \
    }

#define DEFINE_SAFE_CAST_SAME_SIGNEDNESS(f, t)  \
    t f##_to_##t(f from)                        \
    {                                           \
        ASSERT_ALWAYS(from >= MIN_VALUE_OF(t)); \
        ASSERT_ALWAYS(from <= MAX_VALUE_OF(t)); \
        t result = (t)from;                     \
        return result;                          \
    }

DEFINE_SAFE_CAST_SIGNED_TO_UNSIGNED(s64, s32)
DEFINE_SAFE_CAST_SIGNED_TO_UNSIGNED(s32, u32)
DEFINE_SAFE_CAST_SIGNED_TO_UNSIGNED(s32, usize)
DEFINE_SAFE_CAST_UNSIGNED_TO_SIGNED(u32, s32)
DEFINE_SAFE_CAST_UNSIGNED_TO_SIGNED(usize, s32)
DEFINE_SAFE_CAST_UNSIGNED_TO_SIGNED(u64, s32)

#undef DEFINE_SAFE_CAST_SIGNED_TO_UNSIGNED
#undef DEFINE_SAFE_CAST_UNSIGNED_TO_SIGNED
#undef DEFINE_SAFE_CAST_SAME_SIGNEDNESS

/* Wrapping casts */
s32 wrap_u32_to_s32(u32 from)
{
    s32 result = 0;
    copy_mem(&result, &from, 1, sizeof(result));

    return result;
}

/* Safe arithmetic */
#define DEFINE_SAFE_SIGNED_ADD(t)               \
    b32 safe_add_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        t min = MIN_VALUE_OF(t);                \
        t max = MAX_VALUE_OF(t);                \
                                                \
        if ((a == 0) || (b == 0)) {             \
            result = true;                      \
        } else {                                \
            if (a > 0) {                        \
                if (b > 0) {                    \
                    result = a <= (max - b);    \
                } else {                        \
                    result = a >= (min - b);    \
                }                               \
            } else {                            \
                if (b > 0) {                    \
                    result = b >= (min - a);    \
                } else {                        \
                    result = a >= (min - b);    \
                }                               \
            }                                   \
        }                                       \
                                                \
        if (result) {                           \
            *res = a + b;                       \
        }                                       \
                                                \
        return result;                          \
    }

// TODO: can this be simplified?
#define DEFINE_SAFE_SIGNED_SUB(t)                   \
    b32 safe_sub_##t(t a, t b, t *res)              \
    {                                               \
        b32 result = false;                         \
                                                    \
        t min = MIN_VALUE_OF(t);                    \
        t max = MAX_VALUE_OF(t);                    \
                                                    \
        if (a > 0) {                                \
            if (b > 0) {                            \
                result = a >= (min + b);            \
            } else {                                \
                result = a <= (max + b);            \
            }                                       \
        } else {                                    \
            if (b > 0) {                            \
                result = a >= (min + b);            \
            } else {                                \
                result = a <= (max + b);            \
            }                                       \
        }                                           \
                                                    \
        if (result) {                               \
            *res = a - b;                           \
        }                                           \
                                                    \
        return result;                              \
    }

#define DEFINE_SAFE_SIGNED_MUL(t)               \
    b32 safe_mul_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        t min = MIN_VALUE_OF(t);                \
        t max = MAX_VALUE_OF(t);                \
                                                \
        if ((a == 0) || (b == 0)) {             \
            result = true;                      \
        } else if (a > 0) {                     \
            if (b > 0) {                        \
                result = a <= (max / b);        \
            } else {                            \
                result = b >= (min / a);        \
            }                                   \
        } else {                                \
            if (b > 0) {                        \
                result = a >= (min / b);        \
            } else {                            \
                result = b >= (max / a);        \
            }                                   \
        }                                       \
                                                \
        if (result) {                           \
            *res = a * b;                       \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_SIGNED_DIV(t)               \
    b32 safe_div_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        t min = MIN_VALUE_OF(t);                \
                                                \
        if (b == 0) {                           \
            result = false;                     \
        } else if ((a == min) && (b == -1)) {   \
            result = false;                     \
        } else {                                \
            result = true;                      \
        }                                       \
                                                \
        if (result) {                           \
            *res = a / b;                       \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_SIGNED_NEG(t)               \
    b32 safe_neg_##t(t a, t *res)               \
    {                                           \
        b32 result = false;                     \
                                                \
        t min = MIN_VALUE_OF(t);                \
                                                \
        result = a != min;                      \
                                                \
        if (result) {                           \
            *res = -a;                          \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_SIGNED_ABS(t)               \
    b32 safe_abs_##t(t a, t *res)               \
    {                                           \
        b32 result = false;                     \
                                                \
        if (a < 0) {                            \
            result = safe_neg_##t(a, res);      \
        } else {                                \
            *res = a;                           \
            result = true;                      \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_UNSIGNED_ADD(t)             \
    b32 safe_add_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        t max = MAX_VALUE_OF(t);                \
                                                \
        if ((a == 0) || (b == 0)) {             \
            result = true;                      \
        } else {                                \
            result = a <= (max - b);            \
        }                                       \
                                                \
        if (result) {                           \
            *res = a + b;                       \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_UNSIGNED_SUB(t)             \
    b32 safe_sub_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        if (b == 0) {                           \
            result = true;                      \
        } else {                                \
            result = a >= b;                    \
        }                                       \
                                                \
        if (result) {                           \
            *res = a - b;                       \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_UNSIGNED_MUL(t)             \
    b32 safe_mul_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        t max = MAX_VALUE_OF(t);                \
                                                \
        if ((a == 0) || (b == 0)) {             \
            result = true;                      \
        } else {                                \
            result = a <= (max / b);            \
        }                                       \
                                                \
        if (result) {                           \
            *res = a * b;                       \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_UNSIGNED_DIV(t)             \
    b32 safe_div_##t(t a, t b, t *res)          \
    {                                           \
        b32 result = false;                     \
                                                \
        if (b == 0) {                           \
            result = false;                     \
        } else {                                \
            result = true;                      \
        }                                       \
                                                \
        if (result) {                           \
            *res = a / b;                       \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAFE_SIGNED_OPS(t)               \
    DEFINE_SAFE_SIGNED_ADD(t)                   \
    DEFINE_SAFE_SIGNED_SUB(t)                   \
    DEFINE_SAFE_SIGNED_MUL(t)                   \
    DEFINE_SAFE_SIGNED_DIV(t)                   \
    DEFINE_SAFE_SIGNED_NEG(t)                   \
    DEFINE_SAFE_SIGNED_ABS(t)


#define DEFINE_SAFE_UNSIGNED_OPS(t)               \
    DEFINE_SAFE_UNSIGNED_ADD(t)                   \
    DEFINE_SAFE_UNSIGNED_SUB(t)                   \
    DEFINE_SAFE_UNSIGNED_MUL(t)                   \
    DEFINE_SAFE_UNSIGNED_DIV(t)

DEFINE_SAFE_SIGNED_OPS(s32)
DEFINE_SAFE_SIGNED_OPS(s64)
DEFINE_SAFE_SIGNED_OPS(ssize)

DEFINE_SAFE_UNSIGNED_OPS(u32)
DEFINE_SAFE_UNSIGNED_OPS(u64)
DEFINE_SAFE_UNSIGNED_OPS(usize)

#undef DEFINE_SAFE_SIGNED_ADD
#undef DEFINE_SAFE_SIGNED_SUB
#undef DEFINE_SAFE_SIGNED_MUL
#undef DEFINE_SAFE_SIGNED_NEG

#undef DEFINE_SAFE_UNSIGNED_ADD
#undef DEFINE_SAFE_UNSIGNED_SUB
#undef DEFINE_SAFE_UNSIGNED_MUL

#undef DEFINE_SAFE_SIGNED_OPS
#undef DEFINE_SAFE_UNSIGNED_OPS

// TODO: UNLIKELY()
/* Saturating arithmetic */
#define DEFINE_SAT_SIGNED_ADD(t)                \
    t sat_add_##t(t a, t b)                     \
    {                                           \
        t result = 0;                           \
                                                \
        if (!safe_add_##t(a, b, &result)) {     \
            ASSERT(a != 0);                     \
            ASSERT(b != 0);                     \
                                                \
            if (a > 0) {                        \
                ASSERT(b > 0);                  \
                                                \
                result = MAX_VALUE_OF(t);       \
            } else {                            \
                ASSERT(b < 0);                  \
                                                \
                result = MIN_VALUE_OF(t);       \
            }                                   \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_SIGNED_SUB(t)                \
    t sat_sub_##t(t a, t b)                     \
    {                                           \
        t result = 0;                           \
                                                \
        if (!safe_sub_##t(a, b, &result)) {     \
            ASSERT(a != 0);                     \
            ASSERT(b != 0);                     \
                                                \
            if (a > 0) {                        \
                ASSERT(b < 0);                  \
                result = MAX_VALUE_OF(t);       \
            } else {                            \
                ASSERT(b > 0);                  \
                result = MIN_VALUE_OF(t);       \
            }                                   \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_SIGNED_MUL(t)                \
    t sat_mul_##t(t a, t b)                     \
    {                                           \
        t result = 0;                           \
                                                \
        if (!safe_mul_##t(a, b, &result)) {     \
            ASSERT(a != 0);                     \
            ASSERT(b != 0);                     \
                                                \
            if (a > 0) {                        \
                if (b > 0) {                    \
                    result = MAX_VALUE_OF(t);   \
                } else {                        \
                    result = MIN_VALUE_OF(t);   \
                }                               \
            } else {                            \
                if (b > 0) {                    \
                    result = MIN_VALUE_OF(t);   \
                } else {                        \
                    result = MAX_VALUE_OF(t);   \
                }                               \
            }                                   \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_SIGNED_DIV(t)                \
    t sat_div_##t(t a, t b)                     \
    {                                           \
        t result = 0;                           \
                                                \
        if (!safe_div_##t(a, b, &result)) {     \
            if (b == -1) {                      \
                ASSERT(a == MIN_VALUE_OF(t));   \
                result = MAX_VALUE_OF(t);       \
            }                                   \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_UNSIGNED_ADD(t)              \
    t sat_add_##t(t a, t b)                     \
    {                                           \
        t result = a + b;                       \
                                                \
        if (result < a) {                       \
            result = MAX_VALUE_OF(t);           \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_UNSIGNED_SUB(t)              \
    t sat_sub_##t(t a, t b)                     \
    {                                           \
        t result = a - b;                       \
                                                \
        if (result > a) {                       \
            result = MIN_VALUE_OF(t);           \
        }                                       \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_UNSIGNED_MUL(t)                  \
    t sat_mul_##t(t a, t b)                         \
    {                                               \
        t result = a * b;                           \
                                                    \
        if ((result < a) && (a != 0) && (b != 0)) { \
            result = MAX_VALUE_OF(t);               \
        }                                           \
                                                    \
        return result;                              \
    }

#define DEFINE_SAT_UNSIGNED_DIV(t)              \
    t sat_div_##t(t a, t b)                     \
    {                                           \
        t result = 0;                           \
                                                \
        safe_div_##t(a, b, &result);            \
                                                \
        return result;                          \
    }

#define DEFINE_SAT_SIGNED_OPS(t)                \
    DEFINE_SAT_SIGNED_ADD(t)                    \
    DEFINE_SAT_SIGNED_SUB(t)                    \
    DEFINE_SAT_SIGNED_MUL(t)                    \
    DEFINE_SAT_SIGNED_DIV(t)

#define DEFINE_SAT_UNSIGNED_OPS(t)              \
    DEFINE_SAT_UNSIGNED_ADD(t)                  \
    DEFINE_SAT_UNSIGNED_SUB(t)                  \
    DEFINE_SAT_UNSIGNED_MUL(t)                  \
    DEFINE_SAT_UNSIGNED_DIV(t)

DEFINE_SAT_SIGNED_OPS(s64)
DEFINE_SAT_SIGNED_OPS(ssize)

DEFINE_SAT_UNSIGNED_OPS(u64)
DEFINE_SAT_UNSIGNED_OPS(usize)

#undef DEFINE_SAT_SIGNED_ADD
#undef DEFINE_SAT_SIGNED_SUB
#undef DEFINE_SAT_SIGNED_MUL
#undef DEFINE_SAT_SIGNED_DIV

#undef DEFINE_SAT_UNSIGNED_ADD
#undef DEFINE_SAT_UNSIGNED_SUB
#undef DEFINE_SAT_UNSIGNED_MUL
#undef DEFINE_SAT_UNSIGNED_DIV

#undef DEFINE_SAT_SIGNED_OPS
#undef DEFINE_SAT_UNSIGNED_OPS

/* Math */
b32 is_pow2(u64 val)
{
    b32 result = (val != 0) && (((val - 1) & val) == 0);

    return result;
}

b32 is_aligned(u64 val, u64 align)
{
    b32 result = (val % align) == 0;

    return result;
}

b32 ptr_is_aligned(void *val, u64 align)
{
    b32 result = is_aligned((u64)val, align);

    return result;
}

b32 is_aligned_pow2(u64 val, u64 align)
{
    ASSERT(is_pow2(align));

    b32 result = (val & (align - 1)) == 0;

    return result;
}

u64 align_pow2(u64 val, u64 align)
{
    ASSERT(is_pow2(align));

    u64 result = (val + align - 1) & ~(align - 1);
    ASSERT_ALWAYS(result >= val);

    return result;
}

f32 lerp(f32 a, f32 b, f32 t)
{
    ASSERT(t >= 0.0f);
    ASSERT(t <= 1.0f);

    f32 result = a * (1.0f - t) + b * t;

    return result;
}

/* Memory operations */
void copy_mem(void *dst, void *src, usize item_count, usize item_size)
{
    usize byte_count = 0;

    if (safe_mul_usize(item_count, item_size, &byte_count)) {
        memcpy(dst, src, byte_count);
    } else {
        ASSERT_ALWAYS(false);
    }
}
