#include "base_random.h"

// TODO: move into thread context
static thread_local rng_State g__rng_state;

static u64 rng__splitmix(u64 *state)
{
    *state += (u64)0x9E3779B97F4A7C15ull;

    u64 result = *state;
    result = (result ^ (result >> 30)) * (u64)0xBF58476D1CE4E5B9ull;
    result = (result ^ (result >> 27)) * (u64)0x94D049BB133111EBull;
    result = result ^ (result >> 31);

    return result;
}

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
void rng_seed(u64 seed)
{
    rng_seed_s(&g__rng_state, seed);
}

u32 rng_u32(u32 min, u32 max)
{
    return rng_u32_s(&g__rng_state, min, max);
}

u64 rng_u64(u64 min, u64 max)
{
    return rng_u64_s(&g__rng_state, min, max);
}

/* Functions using user provided RNG state */
void rng_seed_s(rng_State *rng, u64 seed)
{
    for (usize i = 0; i < array_capacity(rng->data); ++i) {
        rng->data[i] = rng__splitmix(&seed);
    }
}

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
