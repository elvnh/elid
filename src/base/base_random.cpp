#include "base_random.h"

static u64 rng__next_u64(rng_State *rng)
{
    ASSERT(
        ((rng->data[0] != 0)
        || (rng->data[1] != 0)
        || (rng->data[2] != 0)
        || (rng->data[3] != 0))
        && "Seed before using random number generator"
    );

    u64 *s = rng->data;

    u64 result = rotate_left(s[1] * 5, 7) * 9;
    u64 x = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= x;
    s[3] = rotate_left(s[3], 45);

    return result;
}

/* Functions using global RNG state */
u32 rng_u32(u32 min, u32 max)
{
    return rng_u32_s(&thread_ctx_get()->rng, min, max);
}

u64 rng_u64(u64 min, u64 max)
{
    return rng_u64_s(&thread_ctx_get()->rng, min, max);
}

/* Functions using user provided RNG state */
u32 rng_u32_s(rng_State *rng, u32 min, u32 max)
{
    u32 result = (u32)rng_u64_s(rng, min, max);

    return result;
}

u64 rng_u64_s(rng_State *rng, u64 min, u64 max)
{
    ASSERT(max > min);

    u64 x = rng__next_u64(rng);

    u64 result = min + (x % (max - min));

    return result;
}
