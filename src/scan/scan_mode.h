#pragma once

enum scan_Mode {
    // Non-comparative scan modes
    SCAN_MODE_EXACT,
    SCAN_MODE_GREATER_THAN,

    // Comparative scan modes
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
scan_Params scan_params_make_gt(scan_Type type, scan_Value cmp_value);
