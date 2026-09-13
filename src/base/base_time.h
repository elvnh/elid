#pragma once

// TODO: improve this

#define ns_to_ms(ns) (u64)(ns / (1000 * 1000))

u64 get_monotonic_clock_ns(void);
u64 get_monotonic_clock_ms(void);
