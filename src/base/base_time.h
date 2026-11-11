#pragma once

#define NS_PER_SEC BILLION(1)

#define ns_to_ms(n) ((n) / MILLION(1))

#define ms_to_us(n) ((n) * THOUSAND(1))
#define ms_to_ns(n) ((n) * MILLION(1))

#define us_to_ns(n) ((n) * THOUSAND(1))

enum ClockResolution {
    CLOCK_RES_MILLISECONDS,
    CLOCK_RES_NANOSECONDS,
};

u64 get_monotonic_clock_ns(void);
u64 get_monotonic_clock_ms(void);
