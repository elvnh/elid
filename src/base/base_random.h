#pragma once

struct rng_State {
    u64 data[4];
};

/* Functions using global RNG state */
void rng_seed(u64 seed);

u32  rng_u32(u32 min, u32 max);
u64  rng_u64(u64 min, u64 max);

/* Functions using user provided RNG state */
void rng_seed_s(rng_State *rng, u64 seed);

u32  rng_u32_s(rng_State *rng, u32 min, u32 max);
u64  rng_u64_s(rng_State *rng, u64 min, u64 max);
