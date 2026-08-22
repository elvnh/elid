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

        case SCAN_MODE_GREATER_THAN: {
            scan_Value arg = scan_params.args[0];
            result = scan_value_cmp_gt(scan_params.type, current_value, arg);
        } break;

        // Comparative scan modes
        case SCAN_MODE_CHANGED: {
            scan_Value prev_value = scan_params.args[0];
            result = !scan_value_cmp_eq(scan_params.type, current_value, prev_value);
        } break;

        case SCAN_MODE_CHANGED_BY: {
            scan_Value prev_value = scan_params.args[0];
            scan_Value changed_by = scan_params.args[1];
            scan_Value expected_value = scan_value_op_add(scan_params.type, prev_value, changed_by);

            result = scan_value_cmp_eq(scan_params.type, current_value, expected_value);
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
        case SCAN_MODE_EXACT:
        case SCAN_MODE_GREATER_THAN: {
            result = false;
        } break;

        case SCAN_MODE_CHANGED:
        case SCAN_MODE_CHANGED_BY: {
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

scan_Params scan_params_make_gt(scan_Type type, scan_Value cmp_value)
{
    scan_Params params = {};
    params.mode = SCAN_MODE_GREATER_THAN;
    params.type = type;
    params.args[0] = cmp_value;

    return params;
}
