#include "scan_mode.h"

b32 scan_compare_values(scan_Value current_value, scan_Params scan_params)
{
    b32 result = false;

    BEGIN_EXHAUSTIVE_SWITCH;
    switch (scan_params.mode) {
        // Non-comparative scan modes
        case SCAN_MODE_EXACT: {
            scan_Value arg = scan_params.args[0];
            result = scan_value_cmp_eq(scan_params.type, current_value, arg);
        } break;

        case SCAN_MODE_UNKNOWN: {
            result = true;
        } break;

        case SCAN_MODE_GREATER_THAN: {
            scan_Value arg = scan_params.args[0];
            result = scan_value_cmp_gt(scan_params.type, current_value, arg);
        } break;

        case SCAN_MODE_LESS_THAN: {
            scan_Value arg = scan_params.args[0];
            result = scan_value_cmp_lt(scan_params.type, current_value, arg);
        } break;


        // Comparative scan modes
        case SCAN_MODE_INCREASED: {
            scan_Value prev_value = scan_params.args[0];

            result = scan_value_cmp_gt(SCAN_TYPE_INT32, current_value, prev_value);
        } break;

        case SCAN_MODE_INCREASED_BY: {
            scan_Value prev_value = scan_params.args[0];
            scan_Value expected_increase = scan_params.args[1];

            scan_Value expected_value = scan_value_op_add(scan_params.type, prev_value, expected_increase);
            result = scan_value_cmp_eq(scan_params.type, current_value, expected_value);
        } break;

        case SCAN_MODE_DECREASED: {
            scan_Value prev_value = scan_params.args[0];

            result = scan_value_cmp_lt(scan_params.type, current_value, prev_value);
        } break;

        case SCAN_MODE_DECREASED_BY: {
            scan_Value prev_value = scan_params.args[0];
            scan_Value expected_decrease = scan_params.args[1];

            scan_Value expected_value = scan_value_op_sub(scan_params.type, prev_value, expected_decrease);
            result = scan_value_cmp_eq(scan_params.type, current_value, expected_value);
        } break;

        case SCAN_MODE_CHANGED: {
            scan_Value prev_value = scan_params.args[0];
            result = !scan_value_cmp_eq(scan_params.type, current_value, prev_value);
        } break;

        case SCAN_MODE_CHANGED_BY: {
            scan_Value prev_value = scan_params.args[0];
            scan_Value expected_change = scan_params.args[1];

            scan_Value expected_increased_value = scan_value_op_add(scan_params.type, prev_value, expected_change);
            scan_Value expected_decreased_value = scan_value_op_sub(scan_params.type, prev_value, expected_change);

            result = scan_value_cmp_eq(scan_params.type, current_value, expected_increased_value)
                || scan_value_cmp_eq(scan_params.type, current_value, expected_decreased_value);
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}


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
        case SCAN_MODE_DECREASED_BY: {
            result = true;
        } break;

        INVALID_DEFAULT_CASE;
    }
    END_EXHAUSTIVE_SWITCH;

    return result;
}

scan_Params scan_params_make_eq(scan_Type type, scan_Value exact_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_EXACT;
    params.type = type;
    params.args[0] = exact_value;

    return params;
}

scan_Params scan_params_make_unknown(scan_Type type)
{
    // TODO: taking type as argument is unneccessary
    scan_Params params = {};
    params.mode = SCAN_MODE_UNKNOWN;
    params.type = type;

    return params;
}

scan_Params scan_params_make_gt(scan_Type type, scan_Value cmp_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_GREATER_THAN;
    params.type = type;
    params.args[0] = cmp_value;

    return params;
}

scan_Params scan_params_make_lt(scan_Type type, scan_Value cmp_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_LESS_THAN;
    params.type = type;
    params.args[0] = cmp_value;

    return params;
}

scan_Params scan_params_make_changed(scan_Type type, scan_Value prev_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_CHANGED;
    params.type = type;
    params.args[0] = prev_value;

    return params;
}

scan_Params scan_params_make_changed_by(scan_Type type, scan_Value prev_value, scan_Value change)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_CHANGED_BY;
    params.type = type;
    params.args[0] = prev_value;
    params.args[1] = change;

    return params;
}

scan_Params scan_params_make_increased(scan_Type type, scan_Value prev_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_INCREASED;
    params.type = type;
    params.args[0] = prev_value;

    return params;
}

scan_Params scan_params_make_increased_by(scan_Type type, scan_Value prev_value, scan_Value increase)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_INCREASED_BY;
    params.type = type;
    params.args[0] = prev_value;
    params.args[1] = increase;

    return params;
}

scan_Params scan_params_make_decreased(scan_Type type, scan_Value prev_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_DECREASED;
    params.type = type;
    params.args[0] = prev_value;

    return params;
}

scan_Params scan_params_make_decreased_by(scan_Type type, scan_Value prev_value, scan_Value decrease)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_DECREASED_BY;
    params.type = type;
    params.args[0] = prev_value;
    params.args[1] = decrease;

    return params;
}
