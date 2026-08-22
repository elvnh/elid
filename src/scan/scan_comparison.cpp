#include "scan_comparison.h"

b32 scan_cmp_match(scan_Value current_value, scan_Comparison cmp)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (cmp.mode) {
        // Non-comparative scan modes
        case SCAN_MODE_EXACT: {
            scan_Value arg = cmp.args[0];
            result = scan_val_eq(cmp.value_type, current_value, arg);
        } break;

        case SCAN_MODE_UNKNOWN: {
            result = true;
        } break;

        case SCAN_MODE_GREATER_THAN: {
            scan_Value arg = cmp.args[0];
            result = scan_val_gt(cmp.value_type, current_value, arg);
        } break;

        case SCAN_MODE_LESS_THAN: {
            scan_Value arg = cmp.args[0];
            result = scan_val_lt(cmp.value_type, current_value, arg);
        } break;


        // Comparative scan modes
        case SCAN_MODE_INCREASED: {
            scan_Value prev_value = cmp.args[0];

            result = scan_val_gt(cmp.value_type, current_value, prev_value);
        } break;

        case SCAN_MODE_INCREASED_BY: {
            scan_Value prev_value = cmp.args[0];
            scan_Value expected_increase = cmp.args[1];

            scan_Value expected_value = scan_val_add(cmp.value_type, prev_value, expected_increase);
            result = scan_val_eq(cmp.value_type, current_value, expected_value);
        } break;

        case SCAN_MODE_DECREASED: {
            scan_Value prev_value = cmp.args[0];

            result = scan_val_lt(cmp.value_type, current_value, prev_value);
        } break;

        case SCAN_MODE_DECREASED_BY: {
            scan_Value prev_value = cmp.args[0];
            scan_Value expected_decrease = cmp.args[1];

            scan_Value expected_value = scan_val_sub(cmp.value_type, prev_value, expected_decrease);
            result = scan_val_eq(cmp.value_type, current_value, expected_value);
        } break;

        case SCAN_MODE_CHANGED: {
            scan_Value prev_value = cmp.args[0];
            result = !scan_val_eq(cmp.value_type, current_value, prev_value);
        } break;

        case SCAN_MODE_CHANGED_BY: {
            scan_Value prev_value = cmp.args[0];
            scan_Value expected_change = cmp.args[1];

            scan_Value expected_increased_value = scan_val_add(cmp.value_type, prev_value, expected_change);
            scan_Value expected_decreased_value = scan_val_sub(cmp.value_type, prev_value, expected_change);

            result = scan_val_eq(cmp.value_type, current_value, expected_increased_value)
                || scan_val_eq(cmp.value_type, current_value, expected_decreased_value);
        } break;

        case SCAN_MODE_UNCHANGED: {
            scan_Value prev_value = cmp.args[0];
            result = scan_val_eq(cmp.value_type, current_value, prev_value);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Comparison scan_cmp_make_exact(scan_ValueType type, scan_Value exact_value)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_EXACT;
    params.value_type = type;
    params.args[0] = exact_value;

    return params;
}

scan_Comparison scan_cmp_make_unknown(scan_ValueType type)
{
    // TODO: taking type as argument is unneccessary
    scan_Comparison params = {};
    params.mode = SCAN_MODE_UNKNOWN;
    params.value_type = type;

    return params;
}

scan_Comparison scan_cmp_make_gt(scan_ValueType type, scan_Value cmp_value)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_GREATER_THAN;
    params.value_type = type;
    params.args[0] = cmp_value;

    return params;
}

scan_Comparison scan_cmp_make_lt(scan_ValueType type, scan_Value cmp_value)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_LESS_THAN;
    params.value_type = type;
    params.args[0] = cmp_value;

    return params;
}

scan_Comparison scan_cmp_make_changed(scan_ValueType type, scan_Value prev_value)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_CHANGED;
    params.value_type = type;
    params.args[0] = prev_value;

    return params;
}

scan_Comparison scan_cmp_make_changed_by(scan_ValueType type, scan_Value prev_value, scan_Value change)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_CHANGED_BY;
    params.value_type = type;
    params.args[0] = prev_value;
    params.args[1] = change;

    return params;
}

scan_Comparison scan_cmp_make_increased(scan_ValueType type, scan_Value prev_value)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_INCREASED;
    params.value_type = type;
    params.args[0] = prev_value;

    return params;
}

scan_Comparison scan_cmp_make_increased_by(scan_ValueType type, scan_Value prev_value, scan_Value increase)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_INCREASED_BY;
    params.value_type = type;
    params.args[0] = prev_value;
    params.args[1] = increase;

    return params;
}

scan_Comparison scan_cmp_make_decreased(scan_ValueType type, scan_Value prev_value)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_DECREASED;
    params.value_type = type;
    params.args[0] = prev_value;

    return params;
}

scan_Comparison scan_cmp_make_decreased_by(scan_ValueType type, scan_Value prev_value, scan_Value decrease)
{
    scan_Comparison params = {};
    params.mode = SCAN_MODE_DECREASED_BY;
    params.value_type = type;
    params.args[0] = prev_value;
    params.args[1] = decrease;

    return params;
}

b32 scan_cmp_mode_is_comparative(scan_Mode mode)
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

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}
