#pragma once

/* Functions using thread-local RNG state */
s32  rng_s32(s32 min, s32 max);

u32  rng_u32(u32 min, u32 max);
u64  rng_u64(u64 min, u64 max);
u64  rng_usize(usize min, usize max);

/* Functions using user provided RNG state */
s32  rng_s32_s(rng_State *rng, s32 min, s32 max);

u32  rng_u32_s(rng_State *rng, u32 min, u32 max);
u64  rng_u64_s(rng_State *rng, u64 min, u64 max);
