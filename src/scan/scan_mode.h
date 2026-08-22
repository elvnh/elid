#pragma once

enum scan_Mode {
    // Non-comparative scan modes
    SCAN_MODE_EXACT,
    SCAN_MODE_UNKNOWN,
    SCAN_MODE_GREATER_THAN,
    SCAN_MODE_LESS_THAN,

    // Comparative scan modes
    SCAN_MODE_INCREASED,
    SCAN_MODE_INCREASED_BY,
    SCAN_MODE_DECREASED,
    SCAN_MODE_DECREASED_BY,
    SCAN_MODE_CHANGED,
    SCAN_MODE_CHANGED_BY,
};

struct scan_Params {
    scan_Mode mode;
    scan_Type type;
    scan_Value args[3];
};

b32         scan_compare_values(scan_Value current_value, scan_Params scan_params);
b32         scan_mode_is_comparative(scan_Mode mode);
scan_Params scan_params_make_eq(scan_Type type, scan_Value exact_value);
scan_Params scan_params_make_unknown(scan_Type type);
scan_Params scan_params_make_gt(scan_Type type, scan_Value cmp_value);
scan_Params scan_params_make_lt(scan_Type type, scan_Value cmp_value);
scan_Params scan_params_make_changed(scan_Type type, scan_Value prev_value);
scan_Params scan_params_make_changed_by(scan_Type type, scan_Value prev_value, scan_Value change);
scan_Params scan_params_make_increased(scan_Type type, scan_Value prev_value);
scan_Params scan_params_make_increased_by(scan_Type type, scan_Value prev_value, scan_Value increase);
scan_Params scan_params_make_decreased(scan_Type type, scan_Value prev_value);
scan_Params scan_params_make_decreased_by(scan_Type type, scan_Value prev_value, scan_Value decrease);
