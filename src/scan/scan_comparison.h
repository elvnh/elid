#pragma once

enum scan_Mode {
    // Non-comparative scan modes, i.e doesn't rely on previous value
    SCAN_MODE_EXACT,
    SCAN_MODE_UNKNOWN,
    SCAN_MODE_GREATER_THAN,
    SCAN_MODE_LESS_THAN,

    // Comparative scan modes, relies on previous value
    SCAN_MODE_INCREASED,
    SCAN_MODE_INCREASED_BY,
    SCAN_MODE_DECREASED,
    SCAN_MODE_DECREASED_BY,
    SCAN_MODE_CHANGED,
    SCAN_MODE_CHANGED_BY,
    SCAN_MODE_UNCHANGED,
};

struct scan_Comparison {
    scan_Mode      mode;
    scan_ValueType value_type;
    scan_Value     args[1];
};

b32             scan_cmp_match(scan_Value current_value, scan_Comparison cmp);
b32             scan_cmp_match_with_previous(scan_Value current_value, scan_Comparison cmp, scan_Value prev_value);
b32             scan_cmp_mode_is_comparative(scan_Mode mode);
scan_Comparison scan_cmp_make_exact(scan_ValueType type, scan_Value exact_value);
scan_Comparison scan_cmp_make_unknown(scan_ValueType type);
scan_Comparison scan_cmp_make_greater_than(scan_ValueType type, scan_Value cmp_value);
scan_Comparison scan_cmp_make_less_than(scan_ValueType type, scan_Value cmp_value);
scan_Comparison scan_cmp_make_changed(scan_ValueType type);
scan_Comparison scan_cmp_make_changed_by(scan_ValueType type, scan_Value change);
scan_Comparison scan_cmp_make_increased(scan_ValueType type);
scan_Comparison scan_cmp_make_increased_by(scan_ValueType type, scan_Value increase);
scan_Comparison scan_cmp_make_decreased(scan_ValueType type);
scan_Comparison scan_cmp_make_decreased_by(scan_ValueType type, scan_Value decrease);
