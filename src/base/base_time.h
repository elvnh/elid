#pragma once

#define ns_to_ms(n) ((n) / MILLION(1))

enum ClockResolution {
    CLOCK_RES_MILLISECONDS,
    CLOCK_RES_NANOSECONDS,
};

u64 get_monotonic_clock_ns(void);
u64 get_monotonic_clock_ms(void);
