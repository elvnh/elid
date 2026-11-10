#pragma once

#define TIMED_FUNCTION() TimedFunction ANONYMOUS_VARIABLE(__func__, get_monotonic_clock_ns(), CLOCK_RES_MILLISECONDS)

struct TimedFunction {
    TimedFunction(const char *func_name, u64 start_ns, ClockResolution res)
    {
        function_name = func_name;
        start_clock_ns = start_ns;
        resolution = res;
    }

    ~TimedFunction()
    {
        u64 now_ns = get_monotonic_clock_ns();
        u64 elapsed_ns = now_ns - start_clock_ns;

        u64 elapsed = 0;
        const char *res_str = 0;

        BEGIN_EXHAUSTIVE_SWITCH;
        switch (resolution) {
            case CLOCK_RES_MILLISECONDS: {
                elapsed = ns_to_ms(elapsed_ns);
                res_str = "milliseconds";
            } break;

            case CLOCK_RES_NANOSECONDS: {
                elapsed = elapsed_ns;
                res_str = "nanoseconds";
            } break;

            INVALID_DEFAULT_CASE;
        }
        END_EXHAUSTIVE_SWITCH;

        printf(
            "[BENCHMARK]\n"
            "Function: %s\n"
            "Elapsed:  %" PRIu64 " %s\n\n", function_name, elapsed, res_str);
    }

    const char *function_name;
    u64 start_clock_ns;
    ClockResolution resolution;
};
