#include "scan_mode.h"

b32 scan_mode_is_comparative(scan_Mode mode)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (mode) {
        // Non-comparative scan modes
        case SCAN_MODE_EXACT:
        case SCAN_MODE_UNKNOWN:
        case SCAN_MODE_GREATER_THAN:
        case SCAN_MODE_LESS_THAN: {
            result = false;
        } break;

        // Comparative scan modes
        case SCAN_MODE_CHANGED:
        case SCAN_MODE_CHANGED_BY:
        case SCAN_MODE_INCREASED:
        case SCAN_MODE_INCREASED_BY:
        case SCAN_MODE_DECREASED:
        case SCAN_MODE_DECREASED_BY:
        case SCAN_MODE_UNCHANGED: {
            result = true;
        } break;

        case SCAN_MODE_COUNT: {
            ASSERT(0);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

const char *scan_mode_to_cstr(scan_Mode mode)
{
    const char *result = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (mode) {
        // Non-comparative scan modes
        case SCAN_MODE_EXACT: {
            result = "Exact";
        } break;

        case SCAN_MODE_UNKNOWN: {
            result = "Unknown";
        } break;

        case SCAN_MODE_GREATER_THAN: {
            result = "Greater than...";
        } break;

        case SCAN_MODE_LESS_THAN: {
            result = "Less than...";
        } break;

        // Comparative scan modes
        case SCAN_MODE_CHANGED: {
            result = "Changed";
        } break;

        case SCAN_MODE_CHANGED_BY: {
            result = "Changed by...";
        } break;

        case SCAN_MODE_INCREASED: {
            result = "Increased";
        } break;

        case SCAN_MODE_INCREASED_BY: {
            result = "Increased by...";
        } break;

        case SCAN_MODE_DECREASED: {
            result = "Decreased";
        } break;

        case SCAN_MODE_DECREASED_BY: {
            result = "Decreased by...";
        } break;

        case SCAN_MODE_UNCHANGED: {
            result = "Unchanged";
        } break;

        INVALID_CASE(SCAN_MODE_COUNT);

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_ModeList scan_mode_get_list()
{
    scan_ModeList result = {};

    for (usize i = 0; i < SCAN_MODE_COUNT; ++i) {
        result.data[result.count++] = (scan_Mode)i;
    }

    return result;
}

u32 scan_mode_get_arity(scan_Mode mode)
{
    u32 result = 0;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (mode) {
        case SCAN_MODE_UNKNOWN:
        case SCAN_MODE_CHANGED:
        case SCAN_MODE_INCREASED:
        case SCAN_MODE_DECREASED:
        case SCAN_MODE_UNCHANGED: {
            result = 0;
        } break;

        case SCAN_MODE_EXACT:
        case SCAN_MODE_GREATER_THAN:
        case SCAN_MODE_LESS_THAN:
        case SCAN_MODE_CHANGED_BY:
        case SCAN_MODE_INCREASED_BY:
        case SCAN_MODE_DECREASED_BY: {
            result = 1;
        } break;

        INVALID_CASE(SCAN_MODE_COUNT);

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}
