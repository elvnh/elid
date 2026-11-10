#pragma once

#define ns_to_ms(n) ((n) / MILLION(1))

u64 get_monotonic_clock_ns(void);
u64 get_monotonic_clock_ms(void);
