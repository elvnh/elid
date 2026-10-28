#pragma once

struct rng_State {
    u64 data[4];
};

void rng_seed(rng_State *rng, u64 seed);
