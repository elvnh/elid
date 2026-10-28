#pragma once

/* Functions using thread-local RNG state */
u32  rng_u32(u32 min, u32 max);
u64  rng_u64(u64 min, u64 max);

/* Functions using user provided RNG state */
u32  rng_u32_s(rng_State *rng, u32 min, u32 max);
u64  rng_u64_s(rng_State *rng, u64 min, u64 max);
