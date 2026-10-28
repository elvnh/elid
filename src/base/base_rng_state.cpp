#include "base_rng_state.h"

static u64 rng__splitmix(u64 *state)
{
    *state += (u64)0x9E3779B97F4A7C15ull;

    u64 result = *state;
    result = (result ^ (result >> 30)) * (u64)0xBF58476D1CE4E5B9ull;
    result = (result ^ (result >> 27)) * (u64)0x94D049BB133111EBull;
    result = result ^ (result >> 31);

    return result;
}

void rng_seed(rng_State *rng, u64 seed)
{
    for (usize i = 0; i < array_capacity(rng->data); ++i) {
        rng->data[i] = rng__splitmix(&seed);
    }
}
